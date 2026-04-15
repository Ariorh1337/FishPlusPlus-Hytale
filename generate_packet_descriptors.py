#!/usr/bin/env python3
"""
Generates packet_descriptors.json from Hytale Java packet classes + C++ PacketRegistry.h

NativeAOT memory layout for managed reference types:
  - offset 0: MethodTable* (8 bytes)
  - GC reference fields (object pointers) first, in declaration order
  - Then value type fields in declaration order, with natural alignment padding

Run once from the repo root, outputs packet_descriptors.json next to this script.
"""

import os
import re
import json
from pathlib import Path

# ──────────────────────────────────────────────────────────────────────────────
# Paths
# ──────────────────────────────────────────────────────────────────────────────
JAVA_PROTOCOL_DIR = Path("B:/Job/dev/_java/Hytale-Server-Unpacked-latest/hytale-server-unpacked/com/hypixel/hytale/protocol")
CPP_REGISTRY      = Path("B:/Job/dev/_java/3d=party-cheats/FishPlusPlus-Hytale-2/Hytale/sdk/Packets/PacketRegistry.h")
OUTPUT_JSON       = Path("B:/Job/dev/_java/3d=party-cheats/FishPlusPlus-Hytale-2/packet_descriptors.json")

# ──────────────────────────────────────────────────────────────────────────────
# Type system
# ──────────────────────────────────────────────────────────────────────────────

# Java primitive types → (json_type, alignment, size)
PRIMITIVES = {
    "int":     ("int32",   4, 4),
    "long":    ("int64",   8, 8),
    "float":   ("float32", 4, 4),
    "double":  ("float64", 8, 8),
    "boolean": ("bool",    1, 1),
    "byte":    ("int8",    1, 1),
    "short":   ("int16",   2, 2),
    "char":    ("int16",   2, 2),
}

# Cache for enum/value-type detection
_type_cache: dict[str, tuple[str, int, int]] = {}

def classify_java_type(type_name: str, protocol_dir: Path) -> tuple[str, int, int]:
    """Returns (json_type, alignment, size).

    Categories:
      ptr     - GC reference (object pointer), 8 bytes
      enum32  - Java enum, 4 bytes
      structN - inlined value type of N bytes (not directly settable)
      primitives - as above
    """
    if type_name in PRIMITIVES:
        return PRIMITIVES[type_name]
    if type_name in _type_cache:
        return _type_cache[type_name]

    # Search for the .java file
    matches = list(protocol_dir.rglob(f"{type_name}.java"))
    if not matches:
        # Unknown type → assume reference (ptr)
        result = ("ptr", 8, 8)
        _type_cache[type_name] = result
        return result

    content = matches[0].read_text(encoding="utf-8", errors="ignore")

    # Java enum?
    if re.search(r'\benum\s+' + re.escape(type_name) + r'\b', content):
        result = ("enum32", 4, 4)
        _type_cache[type_name] = result
        return result

    # Value type (struct)? Hytale uses FIXED_BLOCK_SIZE on structs too,
    # but the real tell is that it implements no interfaces or only
    # serialization helpers. We look for FIXED_BLOCK_SIZE to get the size.
    m = re.search(r'FIXED_BLOCK_SIZE\s*=\s*(\d+)', content)
    if m:
        size = int(m.group(1))
        # Value types are inlined. Use 8-byte alignment if size >= 8.
        align = 8 if size >= 8 else (4 if size >= 4 else (2 if size >= 2 else 1))
        result = (f"struct{size}", align, size)
        _type_cache[type_name] = result
        return result

    # Fallback: GC reference
    result = ("ptr", 8, 8)
    _type_cache[type_name] = result
    return result

# ──────────────────────────────────────────────────────────────────────────────
# C++ PacketRegistry.h parser
# ──────────────────────────────────────────────────────────────────────────────

def parse_cpp_registry(path: Path) -> dict:
    """Returns { 'ClassName': { index, id, direction } }"""
    text = path.read_text(encoding="utf-8", errors="ignore")
    out = {}
    # e.g.  ClientPlaceBlock_C2S = 91,  // ID: 117 [Server-Bound]
    for m in re.finditer(
        r'(\w+?)_(C2S|S2C|BI)\s*=\s*(\d+),?\s*//\s*ID:\s*(\d+)\s*\[([^\]]+)\]',
        text
    ):
        class_name = m.group(1)
        suffix     = m.group(2)
        index      = int(m.group(3))
        pkt_id     = int(m.group(4))
        out[class_name] = {"index": index, "id": pkt_id, "direction": suffix}
    return out

# ──────────────────────────────────────────────────────────────────────────────
# Java packet parser
# ──────────────────────────────────────────────────────────────────────────────

def parse_java_fields(java_file: Path, protocol_dir: Path) -> list[dict] | None:
    """Parse instance fields from a Java packet class. Returns None if not a packet.

    Key rule for NativeAOT layout:
      @Nullable / @Nonnull  → reference type (class) → ptr, 8 bytes
      no annotation, non-primitive → check if struct (FIXED_BLOCK_SIZE) or enum
    """
    content = java_file.read_text(encoding="utf-8", errors="ignore")

    if "implements Packet" not in content:
        return None

    fields = []

    # Capture the optional @Nullable/@Nonnull annotation along with the field
    field_re = re.compile(
        r'(@(?:Nullable|Nonnull)\s+)?'          # group 1: annotation (optional)
        r'public\s+(?!static\b)(?!final\b)'
        r'([\w<>\[\]?]+)\s+(\w+)'               # group 2: type, group 3: name
        r'\s*(?:=\s*[^;]+)?\s*;'
    )
    for m in field_re.finditer(content):
        annotation = m.group(1)
        raw_type   = m.group(2)
        field_name = m.group(3)

        # Skip static / UPPER_CASE constants
        line_start = content.rfind('\n', 0, m.start()) + 1
        line = content[line_start:content.find('\n', m.start())]
        if 'static' in line:
            continue
        if 'final' in line and field_name == field_name.upper():
            continue

        base = re.sub(r'<[^>]*>', '', raw_type).rstrip('[]').strip()

        # @Nullable means definitely a heap reference (can be null) → ptr
        # @Nonnull could still be a value type (enum, struct) → fall through
        if annotation and '@Nullable' in annotation:
            json_type, align, size = "ptr", 8, 8
        else:
            json_type, align, size = classify_java_type(base, protocol_dir)
            # If classify returned ptr but the field is @Nonnull, keep ptr
            # (non-null reference type is still a pointer)

        fields.append({
            "name":      field_name,
            "java_type": raw_type,
            "json_type": json_type,
            "align":     align,
            "size":      size,
        })

    return fields

# ──────────────────────────────────────────────────────────────────────────────
# NativeAOT field layout
# ──────────────────────────────────────────────────────────────────────────────

def align_up(value: int, alignment: int) -> int:
    return (value + alignment - 1) & ~(alignment - 1)

def compute_offsets(fields: list[dict]) -> list[dict]:
    """
    NativeAOT layout rule:
      1. GC reference fields (ptr) first, in declaration order, 8 bytes each
      2. All other fields in declaration order, with natural alignment

    Object header (MethodTable*) is at offset 0, user fields start at 8.
    """
    refs     = [f for f in fields if f["json_type"] == "ptr"]
    non_refs = [f for f in fields if f["json_type"] != "ptr"]

    result = []
    offset = 8  # skip MethodTable*

    for f in refs:
        result.append({
            "name":      f["name"],
            "type":      f["json_type"],
            "offset":    offset,
            "size":      f["size"],
            "settable":  False,
        })
        offset += 8  # all ptrs are 8 bytes

    for f in non_refs:
        offset = align_up(offset, f["align"])
        settable = f["json_type"] not in ("ptr",) and not f["json_type"].startswith("struct")
        result.append({
            "name":      f["name"],
            "type":      f["json_type"],
            "offset":    offset,
            "size":      f["size"],
            "settable":  settable,
        })
        offset += f["size"]

    return result

# ──────────────────────────────────────────────────────────────────────────────
# Main
# ──────────────────────────────────────────────────────────────────────────────

def main():
    print("Parsing C++ PacketRegistry…")
    registry = parse_cpp_registry(CPP_REGISTRY)
    print(f"  Found {len(registry)} packet entries in registry")

    packets_dir = JAVA_PROTOCOL_DIR / "packets"
    output = {}
    skipped = 0

    for java_file in sorted(packets_dir.rglob("*.java")):
        class_name = java_file.stem
        if class_name not in registry:
            skipped += 1
            continue

        fields_raw = parse_java_fields(java_file, JAVA_PROTOCOL_DIR)
        if fields_raw is None:
            skipped += 1
            continue

        fields = compute_offsets(fields_raw)
        reg = registry[class_name]

        if reg["direction"] != "C2S":
            skipped += 1
            continue

        output[class_name] = {
            "index":     reg["index"],
            "id":        reg["id"],
            "direction": reg["direction"],
            "fields":    fields,
        }

    OUTPUT_JSON.write_text(json.dumps(output, indent=2), encoding="utf-8")
    print(f"Generated {len(output)} packets  ({skipped} skipped)  ->  {OUTPUT_JSON}")

    # Print summary of settable-only fields for a few example packets
    for name in ["ClientPlaceBlock", "Pong", "ClientMovement"]:
        if name in output:
            settable = [f for f in output[name]["fields"] if f["settable"]]
            print(f"\n  {name} settable fields:")
            for f in settable:
                print(f"    {f['name']:30s} {f['type']:8s}  offset=0x{f['offset']:02X}")

if __name__ == "__main__":
    main()

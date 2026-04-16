#!/usr/bin/env python3
"""
Generates PacketFieldTable.h from Eric McDonald's hytale_packets.h.

Produces:
  - PKT_TABLE[]     — C2S packet definitions with ALL fields (primitives + ptr with type name)
  - SUB_TYPE_TABLE[] — all non-packet struct definitions used as nested field types

Run once; output goes to Hytale/Features/ActualFeatures/PacketFieldTable.h
"""

import re
from pathlib import Path

ERIC_HEADER  = Path("B:/Job/dev/_java/3d=party-cheats/CheatByEricMD/include/hytale_cheat/hook/windows/x64/hytale_packets.h")
CPP_REGISTRY = Path("B:/Job/dev/_java/3d=party-cheats/FishPlusPlus-Hytale-2/Hytale/sdk/Packets/PacketRegistry.h")
OUTPUT_H     = Path("B:/Job/dev/_java/3d=party-cheats/FishPlusPlus-Hytale-2/Hytale/Features/ActualFeatures/PacketFieldTable.h")

# ─────────────────────────────────────────────────────────────────────────────
# Type system
# ─────────────────────────────────────────────────────────────────────────────

TYPE_MAP = {
    "int":              ("Int32",   4, 4),
    "int32_t":          ("Int32",   4, 4),
    "uint32_t":         ("Int32",   4, 4),
    "int64_t":          ("Int64",   8, 8),
    "uint64_t":         ("Int64",   8, 8),
    "float":            ("Float32", 4, 4),
    "double":           ("Float64", 8, 8),
    "bool":             ("Bool",    1, 1),
    "CompactedBoolean": ("Bool",    1, 1),
    "Byte":             ("Int8",    1, 1),
    "uint8_t":          ("Int8",    1, 1),
    "int8_t":           ("Int8",    1, 1),
    "int16_t":          ("Int16",   2, 2),
    "uint16_t":         ("Int16",   2, 2),
    "short":            ("Int16",   2, 2),
    "char":             ("Int8",    1, 1),
}

FTYPE_CPP = {
    "Int8":    "FType::Int8",
    "Int16":   "FType::Int16",
    "Int32":   "FType::Int32",
    "Int64":   "FType::Int64",
    "Float32": "FType::Float32",
    "Float64": "FType::Float64",
    "Float64": "FType::Float64",
    "Bool":    "FType::Bool",
    "Ptr":     "FType::Ptr",
}

def align_up(v, a):
    return (v + a - 1) & ~(a - 1)

# ─────────────────────────────────────────────────────────────────────────────
# Parse Eric's header
# ─────────────────────────────────────────────────────────────────────────────

def parse_enum_types(src: str) -> dict:
    result = {}
    for m in re.finditer(r'enum\s+(\w+)\s*:\s*(\w+)\s*\{', src):
        underlying = m.group(2)
        result[m.group(1)] = TYPE_MAP.get(underlying, ("Int32", 4, 4))
    return result

def parse_cpp_registry(path: Path) -> dict:
    text = path.read_text(encoding="utf-8", errors="ignore")
    result = {}
    for m in re.finditer(r'(\w+?)_(C2S|S2C|BI)\s*=\s*\d+', text):
        result[m.group(1)] = f"{m.group(1)}_{m.group(2)}"
    return result

def extract_struct_body(src: str, start: int) -> str:
    brace = src.index('{', start)
    depth, i = 0, brace
    while i < len(src):
        if src[i] == '{':   depth += 1
        elif src[i] == '}':
            depth -= 1
            if depth == 0:
                return src[brace+1:i]
        i += 1
    return ""

def parse_fields(body: str, enum_types: dict) -> list:
    fields = []
    field_re = re.compile(
        r'^\s*(?:(?:const|volatile|unsigned|signed)\s+)*'
        r'([\w:<>\*\s]+?)\s*(\*)?'
        r'\s+(\w+)\s*(?:\[[\w\s+*]*\])?\s*;',
        re.MULTILINE
    )
    for m in field_re.finditer(body):
        base_type  = m.group(1).strip()
        is_ptr     = bool(m.group(2))
        field_name = m.group(3)

        if field_name in ('method_table', 'padding') or field_name.startswith('_'):
            continue

        if is_ptr:
            fields.append({"name": field_name, "is_ptr": True,
                           "ftype": "Ptr", "ptr_type": base_type,
                           "align": 8, "size": 8})
        elif base_type in TYPE_MAP:
            ft, al, sz = TYPE_MAP[base_type]
            fields.append({"name": field_name, "is_ptr": False,
                           "ftype": ft, "ptr_type": None,
                           "align": al, "size": sz})
        elif base_type in enum_types:
            ft, al, sz = enum_types[base_type]
            fields.append({"name": field_name, "is_ptr": False,
                           "ftype": ft, "ptr_type": None,
                           "align": al, "size": sz})
        else:
            # Unknown non-ptr type → treat as Ptr (safe fallback)
            fields.append({"name": field_name, "is_ptr": True,
                           "ftype": "Ptr", "ptr_type": base_type,
                           "align": 8, "size": 8})
    return fields

def compute_offsets(fields: list) -> list:
    refs     = [f for f in fields if f["is_ptr"]]
    non_refs = [f for f in fields if not f["is_ptr"]]
    result, offset = [], 8  # skip method_table
    for f in refs:
        result.append({**f, "offset": offset}); offset += 8
    for f in non_refs:
        offset = align_up(offset, f["align"])
        result.append({**f, "offset": offset}); offset += f["size"]
    return result

def parse_all_structs(src: str, enum_types: dict) -> dict:
    """Parse every struct in the header. Returns {name: [fields_with_offsets]}."""
    structs = {}
    for m in re.finditer(r'\bstruct\s+(\w+)\s*\{', src):
        name = m.group(1)
        body = extract_struct_body(src, m.start())
        fields = parse_fields(body, enum_types)
        structs[name] = compute_offsets(fields)
    return structs

# ─────────────────────────────────────────────────────────────────────────────
# Code generation helpers
# ─────────────────────────────────────────────────────────────────────────────

def field_line(f: dict, indent: str = "        ") -> str:
    ptr_type_str = f'"{f["ptr_type"]}"' if f["ptr_type"] else "nullptr"
    ft = FTYPE_CPP.get(f["ftype"], "FType::Int32")
    return f'{indent}{{ "{f["name"]}", 0x{f["offset"]:02X}, {ft}, {ptr_type_str} }},'

# ─────────────────────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────────────────────

def main():
    src        = ERIC_HEADER.read_text(encoding="utf-8", errors="ignore")
    enum_types = parse_enum_types(src)
    registry   = parse_cpp_registry(CPP_REGISTRY)
    all_structs = parse_all_structs(src, enum_types)

    # ── All packets (C2S, S2C, BI) ───────────────────────────────────────────
    pkt_re = re.compile(
        r'//\s*Eric McDonald:\s*This is a (?:server-bound|client-bound|bidirectional) packet with ID (\d+) and name "(\w+)":\s*\n'
        r'\s*struct\s+(\w+)\s*\{'
    )

    packet_struct_names = set()
    packets = []
    for m in pkt_re.finditer(src):
        pkt_id, pkt_name = int(m.group(1)), m.group(3)
        if pkt_name not in registry:
            continue
        packet_struct_names.add(pkt_name)
        fields = all_structs.get(pkt_name, [])
        packets.append({"name": pkt_name, "index": registry[pkt_name],
                        "id": pkt_id, "fields": fields})
        ptr_f  = [f for f in fields if f["is_ptr"]]
        prim_f = [f for f in fields if not f["is_ptr"]]
        print(f"  {pkt_name:45s} id={pkt_id:4d}  "
              f"ptr={len(ptr_f)} prim={len(prim_f)}")

    print(f"\n{len(packets)} C2S packets")

    # ── Sub-types (structs referenced as ptr fields in C2S packets) ───────────
    referenced_types: set[str] = set()
    def collect_refs(fields, visited=None):
        if visited is None: visited = set()
        for f in fields:
            if f["is_ptr"] and f["ptr_type"]:
                t = f["ptr_type"]
                # Strip Array< > and trailing *
                if t.startswith("Array<") and t.endswith(">"):
                    t = t[6:-1]
                    if t.endswith("*"):
                        t = t[:-1]
                
                if t not in visited:
                    if t in all_structs and t not in packet_struct_names:
                        visited.add(t)
                        referenced_types.add(t)
                        collect_refs(all_structs[t], visited)

    for pkt in packets:
        collect_refs(pkt["fields"])

    sub_types = {n: all_structs[n] for n in sorted(referenced_types) if n in all_structs}
    print(f"{len(sub_types)} sub-types referenced")

    # ── Write header ──────────────────────────────────────────────────────────
    lines = [
        "// AUTO-GENERATED by generate_packet_table.py — DO NOT EDIT MANUALLY",
        "#pragma once",
        "#include <cstddef>",
        "#include <cstdint>",
        '#include "sdk/Packets/PacketRegistry.h"',
        "",
        "namespace PacketTable {",
        "",
        "enum class FType : uint8_t {",
        "    Int8, Int16, Int32, Int64, Float32, Float64, Bool,",
        "    Ptr  // pointer to a managed heap object; ptr_type gives its struct name",
        "};",
        "",
        "struct FieldDesc {",
        "    const char* name;",
        "    size_t      offset;",
        "    FType       type;",
        "    const char* ptr_type;  // non-null only for FType::Ptr",
        "};",
        "",
        "// ── C2S packet table ─────────────────────────────────────────────────",
        "struct PacketDef {",
        "    const char* name;",
        "    PacketIndex index;",
        "    FieldDesc   fields[128];",
        "    int         field_count;",
        "};",
        "",
        "inline const PacketDef PKT_TABLE[] = {",
    ]

    for pkt in packets:
        lines.append(f'    {{ "{pkt["name"]}", {pkt["index"]}, {{')
        for f in pkt["fields"]:
            lines.append(field_line(f))
        lines.append(f'    }}, {len(pkt["fields"])} }},')

    lines += [
        "};",
        f"inline constexpr int PKT_TABLE_SIZE = {len(packets)};",
        "",
        "// ── Sub-type table ───────────────────────────────────────────────────",
        "struct SubTypeDef {",
        "    const char* name;",
        "    FieldDesc   fields[128];",
        "    int         field_count;",
        "};",
        "",
        "inline const SubTypeDef SUB_TYPE_TABLE[] = {",
    ]

    for name, fields in sub_types.items():
        lines.append(f'    {{ "{name}", {{')
        for f in fields:
            lines.append(field_line(f))
        lines.append(f'    }}, {len(fields)} }},')

    lines += [
        "};",
        f"inline constexpr int SUB_TYPE_TABLE_SIZE = {len(sub_types)};",
        "",
        "}  // namespace PacketTable",
    ]

    OUTPUT_H.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"\nWrote {OUTPUT_H}")

if __name__ == "__main__":
    main()

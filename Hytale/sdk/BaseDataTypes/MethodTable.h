#pragma once
#include <cstdint>

class MethodTable {
public: // private in official codebase
	struct RelatedTypeUnion {
		union {
			// Kinds.CanonicalEEType
			MethodTable* m_pBaseType;

			// Kinds.ParameterizedEEType
			MethodTable* m_pRelatedParameterType;
		};
	};

	// native code counterpart for _uFlags
	union {
		uint32_t              m_uFlags;
		// lower uint16 of m_uFlags is ComponentSize, when HasComponentSize == true
		// also accessed in asm allocation helpers
		uint16_t              m_usComponentSize;
	};
	uint32_t              m_uBaseSize;
	RelatedTypeUnion      m_RelatedType;
	uint16_t              m_usNumVtableSlots;
	uint16_t              m_usNumInterfaces;
	uint32_t              m_uHashCode;

	void* m_VTable[];  // make this explicit so the binder gets the right alignment

	// after the m_usNumVtableSlots vtable slots, we have m_usNumInterfaces slots of
	// MethodTable*, and after that a couple of additional pointers based on whether the type is
	// finalizable (the address of the finalizer code) or has optional fields (pointer to the compacted
	// fields).

	enum Flags {
		// There are four kinds of EETypes, the three of them regular types that use the full MethodTable encoding
		// plus a fourth kind used as a grab bag of unusual edge cases which are encoded in a smaller,
		// simplified version of MethodTable. See LimitedEEType definition below.
		EETypeKindMask = 0x00030000,

		// GC depends on this bit, this bit must be zero
		CollectibleFlag = 0x00200000,

		HasDispatchMapFlag = 0x00040000,

		IsDynamicTypeFlag = 0x00080000,

		// GC depends on this bit, this type requires finalization
		HasFinalizerFlag = 0x00100000,

		HasSealedVTableEntriesFlag = 0x00400000,

		// GC depends on this bit, this type contain gc pointers
		HasPointersFlag = 0x01000000,

		// This type is generic and one or more of it's type parameters is co- or contra-variant. This only
		// applies to interface and delegate types.
		GenericVarianceFlag = 0x00800000,

		// This type is generic.
		IsGenericFlag = 0x02000000,

		// We are storing a EETypeElementType in the upper bits for unboxing enums
		ElementTypeMask = 0x7C000000,
		ElementTypeShift = 26,

		// The m_usComponentSize is a number (not holding ExtendedFlags).
		HasComponentSizeFlag = 0x80000000,
	};

	enum ExtendedFlags {
		HasEagerFinalizerFlag = 0x0001,
		// GC depends on this bit, this type has a critical finalizer
		HasCriticalFinalizerFlag = 0x0002,
		IsTrackedReferenceWithFinalizerFlag = 0x0004,

		// This MethodTable is for a Byref-like class (TypedReference, Span<T>, ...)
		IsByRefLikeFlag = 0x0010,

		// This type requires 8-byte alignment for its fields on certain platforms (ARM32, WASM)
		RequiresAlign8Flag = 0x1000
	};

	enum FunctionPointerFlags {
		IsUnmanaged = 0x80000000,
		FunctionPointerFlagsMask = IsUnmanaged
	};
};
// ============================================================================
// CannonCruise - CAudioStreamEntity (audio stream entity)
//
// Binary: CannonCruise.exe (32-bit MSVC)
//   FUN_004587d0  factory (CClassFactory::Register "CAudioStreamEntity", size 0x54)
//   FUN_00458960  ctor
//   FUN_00458BD0  property apply (sub_458BD0) - vtable slot
//   FUN_00458B60  HandleEvents (mesaj dispatch)
//   FUN_00458D10  stop/release (m_streamHandle/m_state temizler)
//   sub_40E0B0    slot detach -> clear -> loc_40BF90 register -> list attach
//                 (payload = rec+8 inline string, flags = 0)
//
// Main vtable (PTR_FUN_005efbe8): [0]=dtor(458A30), [1]=apply(458BD0),
//   [2]=HandleEvents(458B60), [3]=458FD0, [4]=45A4B0.
// Sub vtable at +0x18 (PTR_LAB_005efbe0): [0]=458F00, [1]=HandleEvents(458B60),
//   [2]=dtor(458A30), [3]=apply(458BD0).
//
// Note: apply id0/id1/id2 calls message-slot register (loc_40BF90); slot
// machine's full functionality is part of message subsystem (todo). This port
// carries layout + property apply semantics: writes first 8 bytes (short/empty
// string prefix) to slot fields and sets "registered" flag.
// ============================================================================

#ifndef CANNONCRUISE_AUDIOSTREAMENTITY_H
#define CANNONCRUISE_AUDIOSTREAMENTITY_H

#include "rwcore.h"

/**
 * @brief CAudioStreamEntity 0x54 byte 32-bit class layout.
 *
 * Binary ctor (FUN_00458960) zeroing and vtable loads:
 *   +0x00 vtable, +0x04..+0x14 zero, +0x18 sub-vtable, +0x1c = 1,
 *   +0x20/+0x28/+0x30 three 8B message slots (FUN_0040b300 init),
 *   +0x38..+0x48 zero (loopCount/streamHandle/floats/state).
 */
struct CAudioStreamEntity {
    RwUInt32 vtable;          // +0x00 PTR_FUN_005efbe8
    RwUInt32 reserved04;      // +0x04
    RwUInt32 reserved08;      // +0x08
    RwUInt32 reserved0c;      // +0x0c
    RwUInt32 reserved10;      // +0x10
    RwUInt32 reserved14;      // +0x14
    RwUInt32 subVtable;       // +0x18 PTR_LAB_005efbe0
    RwUInt32 subOne;          // +0x1c == 1 (ctor)
    RwUInt32 slot20[2];       // +0x20 SAudioPlayStreamData* slotu (id0)
    RwUInt32 slot28[2];       // +0x28 id1 slotu
    RwUInt32 slot30[2];       // +0x30 id2 slotu
    RwUInt32 loopCount;       // +0x38 id3 (dosya: 0xFFFFFFFF = -1)
    RwUInt32 streamHandle;    // +0x3c (reset on stop)
    float    float0;          // +0x40 id4 (file: 5.0f)
    float    float1;          // +0x44 id5 (file: 0.0f)
    RwUInt32 state;           // +0x48 (reset on stop)
    RwUInt32 reserved4c;      // +0x4c
    RwUInt32 reserved50;      // +0x50
};

// 32-bit class layout guarantee
static_assert(sizeof(CAudioStreamEntity) == 0x54,
              "CAudioStreamEntity must be 0x54 bytes (32-bit layout)");
static_assert(sizeof(RwUInt32) == 4, "32-bit build expected");
static_assert(offsetof(CAudioStreamEntity, subVtable) == 0x18, "layout");
static_assert(offsetof(CAudioStreamEntity, subOne) == 0x1c, "layout");
static_assert(offsetof(CAudioStreamEntity, slot20) == 0x20, "layout");
static_assert(offsetof(CAudioStreamEntity, slot28) == 0x28, "layout");
static_assert(offsetof(CAudioStreamEntity, slot30) == 0x30, "layout");
static_assert(offsetof(CAudioStreamEntity, loopCount) == 0x38, "layout");
static_assert(offsetof(CAudioStreamEntity, streamHandle) == 0x3c, "layout");
static_assert(offsetof(CAudioStreamEntity, float0) == 0x40, "layout");
static_assert(offsetof(CAudioStreamEntity, float1) == 0x44, "layout");
static_assert(offsetof(CAudioStreamEntity, state) == 0x48, "layout");

/**
 * @brief Initializes entity with ctor defaults (FUN_00458960).
 */
void CAudioStreamEntityInit(CAudioStreamEntity *pEntity);

/**
 * @brief Applies 0x0704 property record (sub_458BD0).
 *
 * id0: stop (handle/state=0) + clear slot20; if value first byte != 0,
 *      save string prefix to slot20 (SAudioPlayStreamData* register hook).
 * id1: slot28 <- string prefix (sub_40E0B0: rec+8 payload).
 * id2: slot30 <- string prefix.
 * id3: loopCount = *(u32*)value.
 * id4: float0 = *(float*)value.
 * id5: float1 = *(float*)value.
 * default: FALSE (binary default silent).
 *
 * Slot prefix: max 8 bytes up to NUL. "Registered" status in binary is in
 * slot's 2nd word; in this port derived from prefix's first byte (0x00 = empty).
 *
 * @param pEntity Target entity.
 * @param id      Property identifier.
 * @param pValue  Record value (rec+8; NUL-terminated inline for string).
 * @return TRUE if id was processed.
 */
RwBool CAudioStreamEntityApplyAttribute(CAudioStreamEntity *pEntity,
                                        RwUInt32 id,
                                        const RwUInt8 *pValue);

#endif // CANNONCRUISE_AUDIOSTREAMENTITY_H

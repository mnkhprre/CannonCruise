// ============================================================================
// CannonCruise - CAudioStreamEntity implementation
// Binary: sub_458BD0 (apply), FUN_00458960 (ctor), FUN_00458D10 (stop)
// ============================================================================

#include "AudioStreamEntity.h"

#include <cstring>

namespace {

// Copies short/empty string prefix to slot cache (max 8 bytes, up to NUL).
// Binary calls loc_40BF90 payload register; register flag not in this port —
// "registered" status derived from prefix's first byte (0x00 = empty = not registered).
// Part of full slot machine message subsystem.
void CopySlotPrefix(RwUInt32 slot[2], const RwUInt8 *pValue) {
    RwUInt8 bytes[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    if (pValue) {
        for (RwUInt32 i = 0; i < 8 && pValue[i] != 0; ++i) {
            bytes[i] = pValue[i];
        }
    }
    std::memcpy(slot, bytes, sizeof(bytes));
}

} // namespace

void CAudioStreamEntityInit(CAudioStreamEntity *pEntity) {
    std::memset(pEntity, 0, sizeof(*pEntity));
    pEntity->subOne = 1; // ctor: param_1[7] = 1
}

RwBool CAudioStreamEntityApplyAttribute(CAudioStreamEntity *pEntity,
                                        RwUInt32 id,
                                        const RwUInt8 *pValue) {
    switch (id) {
    case 0: {
        // loc_458C22: stop (sub_458D10: +0x3c/+0x48 = 0), slot20 temizle
        pEntity->streamHandle = 0;
        pEntity->state = 0;
        pEntity->slot20[0] = 0;
        pEntity->slot20[1] = 0;
        // if (rec+8[0] != 0) loc_40BF90(slot20, rec+8, "SAudioPlayStreamData*")
        if (pValue && pValue[0] != 0) {
            CopySlotPrefix(pEntity->slot20, pValue);
        }
        return TRUE;
    }
    case 1:
        // loc_458C68: sub_40E0B0(this=+0x18, slot=+0x28, src=rec+8, 0)
        CopySlotPrefix(pEntity->slot28, pValue);
        return TRUE;
    case 2:
        // loc_458C85: sub_40E0B0(this=+0x18, slot=+0x30, src=rec+8, 0)
        CopySlotPrefix(pEntity->slot30, pValue);
        return TRUE;
    case 3:
        // loc_458CA2: [esi+0x38] = *(u32*)(rec+8)
        std::memcpy(&pEntity->loopCount, pValue, 4);
        return TRUE;
    case 4:
        // loc_458CB3: fld [eax+8]; fstp [esi+0x40]
        std::memcpy(&pEntity->float0, pValue, 4);
        return TRUE;
    case 5:
        // loc_458CC4: fld [eax+8]; fstp [esi+0x44]
        std::memcpy(&pEntity->float1, pValue, 4);
        return TRUE;
    default:
        return FALSE; // binary: def_458C1B sessiz atlar
    }
}

// ============================================================================
// CannonCruise - CWbCameraBehavior property apply (CameraState.cpp)
// Reverse-engineered.
// Binary ref: sub_4C3450 (51 case; default advances silently).
// ============================================================================

#include "CameraState.h"

namespace {

inline float AsFloat(RwUInt32 bits) {
    float f = 0.0f;
    std::memcpy(&f, &bits, 4);
    return f;
}

inline void StoreVec3(float *pDst, const RwUInt8 *pSrc) {
    std::memcpy(pDst, pSrc, 12);
}

} // namespace

RwBool CCameraState::ApplyAttribute(RwUInt32 id, RwUInt32 value) {
    // value = 4-byte value of property record (rec+8); in VEC3 records
    // ApplyAttribute call passes raw buffer instead of value — in single-record
    // API VEC3 ids require first 12 bytes from separate buffer. This port uses
    // single-record contract: VEC3 ids use &value bit pattern itself
    // (MAINMENU test passes 12 bytes directly from file). External callers
    // use ApplyAttributeVec3 for VEC3.
    switch (id) {
    case 0x0:
        return FALSE; // VEC3 — via ApplyAttributeVec3
    case 0x1:
        m_clip1 = value;
        return TRUE;
    case 0x2:
        m_clip2 = value;
        return TRUE;
    case 0x3: {
        float rad = CameraDegToRad(AsFloat(value));
        m_fovRad = rad;
        m_fovRadDup = rad;
        return TRUE;
    }
    case 0x4: {
        float t = std::tan(CameraDegToRad(AsFloat(value)));
        m_projX = t;
        m_projXDup = t;
        return TRUE;
    }
    case 0x5:
        m_flagsA = value;
        return TRUE;
    case 0x6:
        return FALSE; // VEC3
    case 0x7:
        m_param7 = value;
        return TRUE;
    case 0x8:
        return FALSE; // VEC3
    case 0x9:
        m_bProjY = std::tan(CameraDegToRad(AsFloat(value)));
        return TRUE;
    case 0xA: {
        float rad = CameraDegToRad(AsFloat(value));
        m_bFovRad = rad;
        m_bFovRadDup = rad;
        return TRUE;
    }
    case 0xB: {
        float rad = CameraDegToRad(AsFloat(value));
        m_bFovRad2 = rad;
        m_bFovRad2Dup = rad;
        return TRUE;
    }
    case 0xC:
        m_bParam12 = value;
        return TRUE;
    case 0xD:
        m_bParam13 = value;
        return TRUE;
    case 0xE:
        m_bParam14 = CameraDegToRad(AsFloat(value));
        return TRUE;
    case 0xF:
        m_bParam15 = CameraDegToRad(AsFloat(value));
        return TRUE;
    case 0x10:
        return FALSE; // VEC3
    case 0x11:
        return FALSE; // VEC3
    case 0x12:
        m_cParam18 = value;
        return TRUE;
    case 0x13:
        m_cParam19 = value;
        return TRUE;
    case 0x14:
        m_cParam20 = value;
        return TRUE;
    case 0x15:
        m_cProj = std::tan(CameraDegToRad(AsFloat(value)));
        return TRUE;
    case 0x16:
        m_cParam22 = value;
        return TRUE;
    case 0x17:
        m_cParam23 = value;
        return TRUE;
    case 0x18: {
        float rad = CameraDegToRad(AsFloat(value));
        m_cParam24 = rad;
        m_cParam24Dup = rad;
        return TRUE;
    }
    case 0x19: {
        float rad = CameraDegToRad(AsFloat(value));
        m_cParam25 = rad;
        m_cParam25Dup = rad;
        return TRUE;
    }
    case 0x1A:
        m_cParam26 = CameraDegToRad(AsFloat(value));
        return TRUE;
    case 0x1B:
        m_cParam27 = value;
        return TRUE;
    case 0x1C:
        return FALSE; // VEC3
    case 0x1D:
        return FALSE; // VEC3
    case 0x1E:
        m_dParam30 = value;
        return TRUE;
    case 0x1F:
        m_dParam31 = value;
        return TRUE;
    case 0x20:
        m_dParam32 = value;
        return TRUE;
    case 0x21:
        m_dProj = std::tan(CameraDegToRad(AsFloat(value)));
        return TRUE;
    case 0x22:
        m_dParam34 = value;
        return TRUE;
    case 0x23:
        m_dParam35 = value;
        return TRUE;
    case 0x24: {
        float rad = CameraDegToRad(AsFloat(value));
        m_dParam36 = rad;
        m_dParam36Dup = rad;
        return TRUE;
    }
    case 0x25: {
        float rad = CameraDegToRad(AsFloat(value));
        m_dParam37 = rad;
        m_dParam37Dup = rad;
        return TRUE;
    }
    case 0x26:
        m_dParam38 = CameraDegToRad(AsFloat(value));
        return TRUE;
    case 0x27:
        m_dParam39 = value;
        return TRUE;
    case 0x28:
        return FALSE; // VEC3
    case 0x29:
        m_param41 = value;
        return TRUE;
    case 0x2A:
        // id42: counter increment if no guard (binary: +0x114 += 1.0f)
        if (m_guard == 0) {
            m_counter += 1.0f;
        }
        return TRUE;
    case 0x2B:
        m_param43 = value;
        return TRUE;
    case 0x2C:
        m_param44 = value;
        return TRUE;
    case 0x2D:
        m_param45 = value;
        return TRUE;
    case 0x2E:
        m_param46 = value;
        return TRUE;
    case 0x2F:
        m_param47 = value;
        return TRUE;
    case 0x30:
        // id48: guard yoksa +0x12c = 1.0f (edi zaten 1.0f)
        if (m_guard == 0) {
            m_one = 1.0f;
        }
        return TRUE;
    case 0x31:
        m_param49 = value;
        return TRUE;
    case 0x32:
        m_param50 = value;
        return TRUE;
    default:
        // binary: default case advances silently — external API returns FALSE
        return FALSE;
    }
}

// VEC3 property records (id 0,6,8,0x10,0x11,0x1C,0x1D,0x28) — 12-byte
// copy; separate entry because single-record value interface is 4 bytes.
// (sub_4C3450 common queue loc_4C393B: rec+8 -> dest, 3 dword.)
RwBool CCameraStateApplyVec3(CCameraState *pState, RwUInt32 id,
                             const RwUInt8 *pVec3) {
    float *pDst = 0;
    switch (id) {
    case 0x0:
        pDst = pState->m_pos;
        break;
    case 0x6:
        pDst = pState->m_dir;
        break;
    case 0x8:
        pDst = pState->m_bPos;
        break;
    case 0x10:
        pDst = pState->m_bDir;
        break;
    case 0x11:
        pDst = pState->m_cPos;
        break;
    case 0x1C:
        pDst = pState->m_cDir;
        break;
    case 0x1D:
        pDst = pState->m_dPos;
        break;
    case 0x28:
        pDst = pState->m_dDir;
        break;
    default:
        return FALSE;
    }
    StoreVec3(pDst, pVec3);
    return TRUE;
}

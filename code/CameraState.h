#ifndef CANNONCRUISE_CAMERASTATE_H
#define CANNONCRUISE_CAMERASTATE_H

// ============================================================================
// CannonCruise - CWbCameraBehavior property apply (CameraState.h)
// Reverse-engineered.
// Binary ref: sub_4C3450 (vtable PTR_LAB_005f0ac4 slot1 — 51-case
// switch; default advances SILENTLY, no assert).
//
// Apply writes to camera-state object (esi) obtained via sub_452170().
// 4 parameter groups (A: id0-7, B: id8-16, C: id17-27, D: id28-40) +
// misc (id41-50) — each group contains position/direction/FOV/aspect triple.
//
// Transformations (constants from exe: flt_5F0AC0=π, flt_5EF7B8=1/180):
//   RAD  : v * π * (1/180)  (FOV degrees → radians)
//   TAN  : tan(v * π/180)   (projection scale; x87 fptan)
//   VEC3 : 12-byte copy (rec+8 -> dest)
//
// id→offset/direct-write table (see .cpp); id42: guard==0 →
// +0x114 += 1.0f; id48: guard==0 → +0x12C = 1.0f; default: skip.
// ============================================================================

#include <rwcore.h>

#include <cmath>
#include <cstddef>
#include <cstring>

/**
 * @struct CCameraState
 * @brief Binary camera-state object (sub_452170 result, ≥0x140 bytes).
 * 32-bit binary layout modeled exactly with RwUInt32/float fields.
 */
struct CCameraState {
    RwUInt32 m_pad00;         ///< +0x00
    float m_pos[3];           ///< +0x04 (id0 VEC3 — konum)
    RwUInt32 m_clip1;         ///< +0x10 (id1 raw; menu: 0.02f bits)
    RwUInt32 m_clip2;         ///< +0x14 (id2 raw; menu: 0.01f bits)
    float m_fovRad;           ///< +0x18 (id3 RAD)
    float m_projX;            ///< +0x1c (id4 TAN)
    RwUInt32 m_flagsA;        ///< +0x20 (id5 raw)
    float m_dir[3];           ///< +0x24 (id6 VEC3 — direction)
    RwUInt32 m_param7;        ///< +0x30 (id7 raw)
    float m_fovRadDup;        ///< +0x34 (id3 RAD duplicate)
    RwUInt32 m_pad38;         ///< +0x38
    float m_projXDup;         ///< +0x3c (id4 TAN duplicate)
    float m_bPos[3];          ///< +0x40 (id8 VEC3)
    float m_bProjY;           ///< +0x4c (id9 TAN)
    float m_bFovRad;          ///< +0x50 (id10 RAD)
    float m_bFovRad2;         ///< +0x54 (id11 RAD)
    RwUInt32 m_bParam12;      ///< +0x58 (id12 raw)
    RwUInt32 m_bParam13;      ///< +0x5c (id13 raw)
    float m_bParam14;         ///< +0x60 (id14 RAD)
    float m_bParam15;         ///< +0x64 (id15 RAD)
    float m_bDir[3];          ///< +0x68 (id16 VEC3)
    float m_bFovRadDup;       ///< +0x74 (id10 RAD duplicate)
    float m_bFovRad2Dup;      ///< +0x78 (id11 RAD duplicate)
    float m_cPos[3];          ///< +0x7c (id17 VEC3)
    RwUInt32 m_cParam18;      ///< +0x88 (id18 raw)
    RwUInt32 m_cParam19;      ///< +0x8c (id19 raw)
    RwUInt32 m_cParam20;      ///< +0x90 (id20 raw)
    float m_cProj;            ///< +0x94 (id21 TAN)
    RwUInt32 m_cParam22;      ///< +0x98 (id22 raw)
    RwUInt32 m_cParam23;      ///< +0x9c (id23 raw)
    float m_cParam24;         ///< +0xa0 (id24 RAD)
    float m_cParam25;         ///< +0xa4 (id25 RAD)
    float m_cParam26;         ///< +0xa8 (id26 RAD)
    RwUInt32 m_cParam27;      ///< +0xac (id27 raw)
    float m_cDir[3];          ///< +0xb0 (id28 VEC3)
    float m_cParam24Dup;      ///< +0xbc (id24 RAD duplicate)
    float m_cParam25Dup;      ///< +0xc0 (id25 RAD duplicate)
    float m_dPos[3];          ///< +0xc4 (id29 VEC3)
    RwUInt32 m_dParam30;      ///< +0xd0 (id30 raw)
    RwUInt32 m_dParam31;      ///< +0xd4 (id31 raw)
    RwUInt32 m_dParam32;      ///< +0xd8 (id32 raw)
    float m_dProj;            ///< +0xdc (id33 TAN)
    RwUInt32 m_dParam34;      ///< +0xe0 (id34 raw)
    RwUInt32 m_dParam35;      ///< +0xe4 (id35 raw)
    float m_dParam36;         ///< +0xe8 (id36 RAD)
    float m_dParam37;         ///< +0xec (id37 RAD)
    float m_dParam38;         ///< +0xf0 (id38 RAD)
    RwUInt32 m_dParam39;      ///< +0xf4 (id39 raw)
    float m_dDir[3];          ///< +0xf8 (id40 VEC3)
    float m_dParam36Dup;      ///< +0x104 (id36 RAD duplicate)
    float m_dParam37Dup;      ///< +0x108 (id37 RAD duplicate)
    RwUInt32 m_pad10c;        ///< +0x10c
    RwUInt32 m_param41;       ///< +0x110 (id41 raw)
    float m_counter;          ///< +0x114 (id42: guard==0 ise += 1.0f)
    RwUInt32 m_param43;       ///< +0x118 (id43 raw)
    RwUInt32 m_param44;       ///< +0x11c (id44 raw)
    RwUInt32 m_param45;       ///< +0x120 (id45 raw)
    RwUInt32 m_param46;       ///< +0x124 (id46 raw)
    RwUInt32 m_param47;       ///< +0x128 (id47 raw)
    float m_one;              ///< +0x12c (id48: guard==0 ise = 1.0f)
    RwUInt32 m_pad130;        ///< +0x130
    RwUInt32 m_param49;       ///< +0x134 (id49 raw)
    RwUInt32 m_param50;       ///< +0x138 (id50 raw)
    RwUInt32 m_guard;         ///< +0x13c (id42/48 guard)

    /**
     * @brief Applies single CWbCameraBehavior property record.
     * @param id    0x0704 property identifier (0..50).
     * @param value 4-byte raw (bit/RAD/TAN conversion for float fields).
     * @return FALSE only when id > 50 (binary default: silent skip).
     */
    RwBool ApplyAttribute(RwUInt32 id, RwUInt32 value);
};

// Binary float sabitleri (exe .rdata)
//   flt_5F0AC0 = 0x40490FDB (π), flt_5EF7B8 = 0x3BB60B61 (1/180)

/// Degree → radian multiplier (π/180 — binary uses two separate fmuls)
inline float CameraDegToRad(float deg) {
    const float kPi = 3.1415927410125732f; // 0x40490FDB
    const float kOneOver180 = 0.0055555556900799274f; // 0x3BB60B61
    return deg * kPi * kOneOver180;
}

/**
 * @brief Applies VEC3 property records (12-byte copy — loc_4C393B).
 * @param pState Target camera state.
 * @param id     Property identifier (0, 6, 8, 0x10, 0x11, 0x1C, 0x1D, 0x28).
 * @param pVec3  12-byte source (rec+8).
 * @return TRUE if id was a VEC3 record and was copied.
 */
RwBool CCameraStateApplyVec3(CCameraState *pState, RwUInt32 id,
                             const RwUInt8 *pVec3);

// Layout validation — binary byte offsets
static_assert(offsetof(CCameraState, m_pos) == 0x04, "id0 → +0x04");
static_assert(offsetof(CCameraState, m_clip1) == 0x10, "id1 → +0x10");
static_assert(offsetof(CCameraState, m_fovRad) == 0x18, "id3 → +0x18");
static_assert(offsetof(CCameraState, m_projX) == 0x1c, "id4 → +0x1c");
static_assert(offsetof(CCameraState, m_dir) == 0x24, "id6 → +0x24");
static_assert(offsetof(CCameraState, m_projXDup) == 0x3c, "id4 → +0x3c");
static_assert(offsetof(CCameraState, m_bPos) == 0x40, "id8 → +0x40");
static_assert(offsetof(CCameraState, m_bProjY) == 0x4c, "id9 → +0x4c");
static_assert(offsetof(CCameraState, m_cPos) == 0x7c, "id17 → +0x7c");
static_assert(offsetof(CCameraState, m_cDir) == 0xb0, "id28 → +0xb0");
static_assert(offsetof(CCameraState, m_dPos) == 0xc4, "id29 → +0xc4");
static_assert(offsetof(CCameraState, m_dDir) == 0xf8, "id40 → +0xf8");
static_assert(offsetof(CCameraState, m_param41) == 0x110, "id41 → +0x110");
static_assert(offsetof(CCameraState, m_one) == 0x12c, "id48 → +0x12c");
static_assert(offsetof(CCameraState, m_guard) == 0x13c, "guard → +0x13c");
static_assert(sizeof(CCameraState) == 0x140, "kamera durumu 0x140 bayt");

#endif // CANNONCRUISE_CAMERASTATE_H

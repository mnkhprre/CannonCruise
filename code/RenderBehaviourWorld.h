#ifndef CANNONCRUISE_RENDERBEHAVIOURWORLD_H
#define CANNONCRUISE_RENDERBEHAVIOURWORLD_H

// ============================================================================
// CannonCruise - RenderBehaviourWorld singleton (RenderBehaviourWorld.h)
// Tersine mühendislik ile yeniden yazılmıştır.
// Binary ref: FUN_004c4410 (ctor, DAT_00635648 singleton'ı oluşturur),
//             FUN_004c3e40 (WbRender attribute apply — 0x0704 property
//             switch'i, kaynak: D:\Projects\CannonCruisePC\code\...
//             renderbehavior.cpp satır 0xC0).
//
// 0x0704 property id → singleton byte offset haritası (FUN_004c3e40):
//   id 0x0 → +0x14 (float; ctor default 0.0f)
//   id 0x1 → +0x18 (float; ctor default 125.0f)
//   id 0x2 → +0x1c (float; ctor default 1.0f)
//   id 0x3 → +0x5c (u32 raw)
//   id 0x4 → +0x60 (u32 raw)
//   id 0x5 → +0x28 (float raw)
//   id 0x6 → +0x30/+0x34/+0x38 (ARGB → R,G,B baytları float olarak)
//   id 0x7 → +0x2c (float raw)
//   id 0x8 → +0x50 (float; ctor default 1000.0f)
//   id 0x9 → +0x54 (float; ctor default 300.0f)
//   id 0xA → +0x44/+0x48/+0x4c (ARGB → R,G,B float)
//   id 0xB → +0x3c (float raw)
//   id 0xC → +0x40 (float raw)
//   diğer id → assert "Unhandled attribute in WbRender..."
//
// Not: +0x04..+0x1B aralığı binary'de CWbRenderBehavior gömülü sub-object
// (24B: vtable + 5 word); +0x14/+0x18 bu sub-object'in alanlarıdır.
// ============================================================================

#include <rwcore.h>

#include <cstddef>

/// 0x0704 nesne şemasındaki property apply sonucu
enum {
    RENDERATTR_APPLIED = TRUE,   ///< id bilinen, alan yazıldı
    RENDERATTR_UNHANDLED = FALSE ///< id bilinmeyen (binary: assert)
};

/**
 * @struct CRenderBehaviourSub
 * @brief Binary'deki gömülü CWbRenderBehavior sub-object (24B, +0x04).
 * Vtable işaretçisi 32-bit binary düzenini modellemek için RwUInt32.
 */
struct CRenderBehaviourSub {
    RwUInt32 m_uVtable; ///< +0x04 (binary: PTR_LAB_005f0aec)
    RwUInt32 m_w1;      ///< +0x08 (bilinmeyen)
    RwUInt32 m_w2;      ///< +0x0c (bilinmeyen)
    RwUInt32 m_w3;      ///< +0x10 (bilinmeyen)
    float m_param0;     ///< +0x14 (prop id 0; ctor default 0.0f)
    float m_param1;     ///< +0x18 (prop id 1; ctor default 125.0f)
};

/**
 * @struct CRenderBehaviourWorld
 * @brief RenderBehaviourWorld singleton — mesaj güdümlü kamera pass sistemi.
 *
 * Binary ctor'ı (FUN_004c4410) dev bir nesne oluşturur: 12 mesaj→kamera
 * pass kaydı (MSG_MENU_RENDER_RT2D vb.), COcean wire-in (+0x60/+0x68 GUID)
 * ve render pool'ları (120000B + 180000B + ...). Bu port şimdilik yalnızca
 * 0x0704 property apply'nin dokunduğu çekirdek alanı (+0x00..+0x63)
 * birebir modelleyip; pass/pool alanlarını gerektiğinde genişletir.
 *
 * Erişim: CRenderBehaviourWorld::Instance() (binary: DAT_00635648).
 */
struct CRenderBehaviourWorld {
    RwUInt32 m_uVtable;              ///< +0x00 (binary: PTR_FUN_005f0af4)
    CRenderBehaviourSub m_Render;    ///< +0x04 (24B gömülü behaviour)
    float m_param2;             ///< +0x1c (prop id 2; ctor default 1.0f)
    RwUInt32 m_pad20;           ///< +0x20 (ctor: 0)
    RwUInt32 m_pad24;           ///< +0x24 (ctor: 1)
    float m_param5;             ///< +0x28 (prop id 5)
    float m_param7;             ///< +0x2c (prop id 7)
    float m_color1R;            ///< +0x30 (prop id 6, R)
    float m_color1G;            ///< +0x34 (prop id 6, G)
    float m_color1B;            ///< +0x38 (prop id 6, B)
    float m_paramB;             ///< +0x3c (prop id 0xB)
    float m_paramC;             ///< +0x40 (prop id 0xC; menu'de 30.0f)
    float m_color2R;            ///< +0x44 (prop id 0xA, R)
    float m_color2G;            ///< +0x48 (prop id 0xA, G)
    float m_color2B;            ///< +0x4c (prop id 0xA, B)
    float m_param8;             ///< +0x50 (prop id 8; ctor default 1000.0f)
    float m_param9;             ///< +0x54 (prop id 9; ctor default 300.0f)
    RwUInt32 m_pad58;           ///< +0x58 (ctor: 0)
    RwUInt32 m_param3;          ///< +0x5c (prop id 3)
    RwUInt32 m_param4;          ///< +0x60 (prop id 4)

    /**
     * @brief Tekil örneği döndürür; yoksa oluşturur (binary ctor default'ları).
     */
    static CRenderBehaviourWorld *Instance();

    /**
     * @brief Tekil örneği siler (test/teardown).
     */
    static void Destroy();

    /**
     * @brief Binary ctor default'larını (FUN_004c4410) uygular.
     */
    void Init();

    /**
     * @brief Tek bir 0x0704 property kaydını singleton alanına yazar.
     *
     * FUN_004c3e40 switch'ine birebir sadık: id 6/0xA ARGB decode
     * (baytlar float'a çevrilir), diğerleri raw u32 kopya.
     *
     * @param id    Property kimliği (0x0704 kaydının id alanı).
     * @param value Değer (4 bayt raw; float alanlara bit kopyası).
     * @return RENDERATTR_APPLIED veya RENDERATTR_UNHANDLED (id > 0xC).
     */
    RwBool ApplyWbRenderAttribute(RwUInt32 id, RwUInt32 value);

private:
    static CRenderBehaviourWorld *sm_pInstance;
};

// Layout doğrulaması — binary byte offset'leri
static_assert(sizeof(CRenderBehaviourSub) == 0x18,
              "CWbRenderBehavior sub-object 0x18 bayt olmalı");
static_assert(offsetof(CRenderBehaviourWorld, m_Render) == 0x04,
              "sub-object +0x04");
static_assert(offsetof(CRenderBehaviourWorld, m_param2) == 0x1c,
              "prop id 2 → +0x1c");
static_assert(offsetof(CRenderBehaviourWorld, m_param5) == 0x28,
              "prop id 5 → +0x28");
static_assert(offsetof(CRenderBehaviourWorld, m_param7) == 0x2c,
              "prop id 7 → +0x2c");
static_assert(offsetof(CRenderBehaviourWorld, m_color1R) == 0x30,
              "prop id 6 RGB → +0x30");
static_assert(offsetof(CRenderBehaviourWorld, m_paramB) == 0x3c,
              "prop id 0xB → +0x3c");
static_assert(offsetof(CRenderBehaviourWorld, m_paramC) == 0x40,
              "prop id 0xC → +0x40");
static_assert(offsetof(CRenderBehaviourWorld, m_color2R) == 0x44,
              "prop id 0xA RGB → +0x44");
static_assert(offsetof(CRenderBehaviourWorld, m_param8) == 0x50,
              "prop id 8 → +0x50");
static_assert(offsetof(CRenderBehaviourWorld, m_param9) == 0x54,
              "prop id 9 → +0x54");
static_assert(offsetof(CRenderBehaviourWorld, m_param3) == 0x5c,
              "prop id 3 → +0x5c");
static_assert(offsetof(CRenderBehaviourWorld, m_param4) == 0x60,
              "prop id 4 → +0x60");

#endif // CANNONCRUISE_RENDERBEHAVIOURWORLD_H

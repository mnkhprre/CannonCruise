// ============================================================================
// CannonCruise - Render Pipeline state (RenderPipeline.h)
// Original path: D:\Projects\CannonCruisePC\code\RenderPipeline.h
// Tersine mühendislik ile CC_ida.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004cc1f0 — RenderPipeline_Reset       (Im2D/matris state sıfırlama)
//   FUN_004cbd20 — RenderPipeline_MatrixIdentity (üst matrisi birim yap)
//   FUN_004cc2f0 — RenderPipeline_MatrixPush  (matris yığını push)
//   FUN_004cc350 — RenderPipeline_MatrixPushCopy (src × üst push)
//   FUN_004cc480 — RenderPipeline_MatrixScaleXY  (üst matrisi (x,y,1) ölçekle)
//   FUN_004cc4d0 — RenderPipeline_MatrixTranslateXY ((x,y,0) ötele)
//   FUN_004cc020 — RenderPipeline_SetViewWindow (görüş penceresi/en-boy —
//                  kamera nesnesi erişimi olmayan taşınabilir çekirdek)
//                  + RenderPipeline_SetViewWindowFromCamera (tam yol:
//                  kamera alanları + önbellek geçersiz kılma + en-boy)
//   FUN_004cc120 — RenderPipeline_GetCamera   (aktif kamera önbelleği)
//   FUN_004cc130 — RenderPipeline_SetContext   (dword_66d79c bağlam ptr)
//   FUN_004cbe10 — RenderPipeline_GetViewParams (pencere ofseti + en-boy)
//   FUN_004cc520 — RenderPipeline_BuildViewMatrix (projeksiyon param
//                  bloğu: önbellekli ölçek matrisi + tersi + kamera
//                  frame çarpımı)
//   FUN_004cc140 — RenderPipeline_Init        (startup alt-sistem zinciri;
//                  renderpipeline.cpp'te mevcut, bu modül reset adımını
//                  sağlar)
//
// Yerleşim: binary RwMatrix 64 bayt (right/up/at/pos + flags) — CCMatrix
// birebir aynı. Yığın: dword_66D644[0..31] slot işaretçileri, dword_66D6C4
// derinlik (0 tabanlı üst); slot[N] = slot[N-1] kopyası (sub_4CC2F0).
// Kök/global matris dword_66D604 (flags dword_66D610 |= 0x20003).
//
// Motor tarafı (portlanmadı — engine bağlamı gerekir):
//   sub_4d5450/sub_4d0e70/sub_4d41c0 — 8/0x14/0x10/0x1C bayt akış kayıtları
//   (sub_50dbf0) + freelist (dword_65d7a4 vtable +0x144 tahsisi); portta
//   karşılığı havuz tahsisi (PoolAlloc) — kayıt no-op.
//   sub_4cc020 vtable kancası ([kamera+0x1C] → sub_4cc110 thunk,
//   orijinal dword_66d818'e) — motor nesnesi yazar; portta atlantır
//   (oyun kamera güncellemesinde SetViewWindowFromCamera'yı çağırır).
//   sub_50e990 (RwFrameGetMatrix — frame+0x50 LTM) — port RCCCameraView
//   pFrameMatrix alanıyla doğrudan matris verir.
//
// sub_4cc520 param bloğu (taşınabilir): önbellekli ölçek matrisi
// unk_66d6d8 = (üst × kök) · ölçek(zScale,zScale,1) · ölçek(-w,w,1) ·
// translate(winX2×zScale×0.5, -winY2×zScale×0.5, 0); tersi unk_66d718
// (sub_50ff20); sonuç unk_66d758 = ölçek × kamera frame (her çağrıda).
// ============================================================================
#pragma once

#include "rwcore.h"

/**
 * @brief 64 baytlık 3x4 afin matris — binary RwMatrix yerleşimi.
 * right/up/at sütunları (taban vektörleri) + flags; pos = öteleme.
 */
struct CCMatrix {
    RwReal right[3];  ///< +0x00 sütun 0 (X taban)
    RwInt32 flags;    ///< +0x0C (binary: |= 0x20003 — birimlik işaretleri)
    RwReal up[3];     ///< +0x10 sütun 1 (Y taban)
    RwInt32 pad0;     ///< +0x1C
    RwReal at[3];     ///< +0x20 sütun 2 (Z taban)
    RwInt32 pad1;     ///< +0x2C
    RwReal pos[3];    ///< +0x30 öteleme
    RwInt32 pad2;     ///< +0x3C
};

// RenderPipeline_Reset — FUN_004cc1f0. Yığın derinliği 0, slot[0] birim
// matris, kök matris (dword_66d604) birim, indeks tablosu kimlik, en-boy
// çifti 1.0, z (dword_66d7a0) = 1.0, half-pixel (dword_66d7a4) = 0.5,
// önbellek bayrakları temiz. Kamera/pencere parametreleri sonradan
// RenderPipeline_SetViewWindow ile gelir (yoksa 1.0 kalır — binary gibi).
RwBool RenderPipeline_Reset(void);

// RenderPipeline_MatrixIdentity — FUN_004cbd20. Üst matrisi birim yapar,
// flags |= 0x20003, önbellek bayraklarını temizler.
RwBool RenderPipeline_MatrixIdentity(void);

// RenderPipeline_MatrixPush — FUN_004cc2f0. Derinlik < 31 ise üst'e bir
// seviye ekler ve ebeveynin kopyasını yazar; taşma → FALSE (üst değişmez).
RwBool RenderPipeline_MatrixPush(void);

// RenderPipeline_MatrixPushCopy — FUN_004cc350. pSrc == NULL → MatrixPush
// (ebeveyn kopyası); değilse yeni üst = pSrc × ebeveyn (sub_50f760 çarpımı,
// ebeveyn önce uygulanır).
RwBool RenderPipeline_MatrixPushCopy(const CCMatrix *pSrc);

// RenderPipeline_MatrixScaleXY — FUN_004cc480. Üst matris tabanlarını
// (x, y, 1) ile ölçekler (RwMatrixScale — pos'a dokunmaz); önbellek sıfırla.
RwBool RenderPipeline_MatrixScaleXY(RwReal x, RwReal y);

// RenderPipeline_MatrixTranslateXY — FUN_004cc4d0. pos += right×x + up×y
// (RwMatrixTranslate — öteleme mevcut ölçek/döndürmeden geçirilir);
// önbellek sıfırla.
RwBool RenderPipeline_MatrixTranslateXY(RwReal x, RwReal y);

// RenderPipeline_SetViewWindow — FUN_004cc020 taşınabilir çekirdeği.
// winX2/winY2 = kamera görüş penceresi ×2 (binary [kamera+0x68/+0x6C]×2);
// en-boy çifti (flt_66d7c0, flt_66d7c4): büyüğü/bir küçük = diğeri 1.0.
// width/height = raster piksel boyutları (flt_66d6d0/d4 — bilgi amaçlı).
RwBool RenderPipeline_SetViewWindow(RwReal width, RwReal height,
                                    RwReal winX2, RwReal winY2);

/**
 * @brief Binary RwCamera için taşınabilir kamera görünümü — sub_4cc020/
 * sub_4cc520'nin okuduğu alanlar; ofsetler binary yerleşimini belgeler.
 */
struct RCCCameraView {
    RwInt32 projFlag;             ///< +0x14 — ==2 → derinlik z 1.0 (değilse
                                  ///< dword_66d7a0); rwCAMERAPROJECTION
    RwInt32 rasterWidth;          ///< [raster+0xC] — fild → flt_66d6d0
    RwInt32 rasterHeight;         ///< [raster+0x10] — fild → flt_66d6d4
    RwReal viewWindowX;           ///< +0x68 — yarı genişlik (×2 → flt_66d6c8)
    RwReal viewWindowY;           ///< +0x6C — yarı yükseklik (×2 → flt_66d6cc)
    const CCMatrix *pFrameMatrix; ///< sub_50e990([kamera+4]) → frame LTM
};

// RenderPipeline_SetViewWindowFromCamera — FUN_004cc020 tam yolu.
// Kamera NULL → aktif kamera önbelleği temizlenir, FALSE; raster NULL →
// FALSE (önbellek korunur — binary gibi). Başarılı: SetViewWindow çekirdeği
// + aktif kamera kaydı + pencere ofsetleri sıfır + sub_4cc520 önbellekleri
// geçersiz. vtable kancası atlantır (motor tarafı — başlık bloğu).
RwBool RenderPipeline_SetViewWindowFromCamera(const RCCCameraView *pCam);

// RenderPipeline_GetCamera — FUN_004cc120. dword_66d7cc aktif kamera
// önbelleği (SetViewWindowFromCamera kaydeder; Reset temizler).
const RCCCameraView *RenderPipeline_GetCamera(void);

// RenderPipeline_SetContext — FUN_004cc130. dword_66d79c — oyunun
// geçerli çizim bağlamı (scene/root); FUN_004cc140 ilk adımı sıfırlar.
RwBool RenderPipeline_SetContext(void *pContext);
void RenderPipeline_ClearContext(void); // dword_66d79c = 0
void *RenderPipeline_GetContext(void);

// RenderPipeline_GetViewParams — FUN_004cbe10. pencere ofseti (dword_
// 66d7b8/bc — SetViewWindowFromCamera sıfırlar) + en-boy çifti.
void RenderPipeline_GetViewParams(RwReal *pOffX, RwReal *pOffY,
                                  RwReal *pAspectX, RwReal *pAspectY);

// RenderPipeline_BuildViewMatrix — FUN_004cc520. Önbellekli ölçek
// matrisi (unk_66d6d8) geçersizse yeniden kurar (üst × kök · ölçek(zScale,
// zScale, 1) · ölçek(-w,w,1) · translate(winX2×zScale×0.5, -winY2×zScale×
// 0.5, 0); tersi unk_66d718. Sonuç unk_66d758 = ölçek × kamera frame —
// HER ÇAĞRIDA (kamera frame'i önbellek tazeler). Kamera/kare matrisi yoksa
// port kılavuzu: ölçek matrisinin kendisi döner (binary kilitlerdi).
const CCMatrix *RenderPipeline_BuildViewMatrix(void);

// Test erişimi — üst matris (dword_66d644[dword_66d6c4]) ve derinlik
// (dword_66d6c4); en-boy çifti (flt_66d7c0/66d7c4).
const CCMatrix *RenderPipeline_GetTopMatrix(void);
RwInt32 RenderPipeline_GetStackDepth(void);
void RenderPipeline_GetAspect(RwReal *pAspectX, RwReal *pAspectY);
void RenderPipeline_GetViewWindow(RwReal *pWidth, RwReal *pHeight,
                                  RwReal *pWinX2, RwReal *pWinY2);
const CCMatrix *RenderPipeline_GetViewScaleMatrix(void); // unk_66d6d8
const CCMatrix *RenderPipeline_GetViewInverseMatrix(void); // unk_66d718

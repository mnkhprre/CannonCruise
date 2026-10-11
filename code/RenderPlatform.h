#ifndef CANNONCRUISE_RENDERPLATFORM_H
#define CANNONCRUISE_RENDERPLATFORM_H

// ============================================================================
// CannonCruise - Render Platformu (RenderPlatform.h)
// Faz 4.6n: librw tabanlı sahne platformu — engine running iken dünya +
// kamera + ışık kurar; kare başlangıç/bitiş zinciri (clear → BeginUpdate →
// uygulama çizimi → EndUpdate → ShowRaster) ve Im2D test karesi sağlar.
//
// Bağlantı: librw C++ API (rw::World/rw::Camera/rw::Light — rw_librw.cpp
// gibi). Stub C API (rwstubs.cpp) KULLANILMAZ — oradaki kamera/world
// fonksiyonları no-op'tur. NULL platformda tüm device çağrıları no-op
// (headless smoke); GL3 platformda gerçek piksel üretir (ShowRaster,
// frameBuffer nil ise atlanır).
//
// Kare döngüsü binary karşılığı: sub_4189E0 (MainLoop) / FUN_0041C380
// (Idle) / sub_41CEB0 (Render — Begin/EndUpdate çift sarmalı).
// ============================================================================

#include "rwcore.h"

/**
 * @brief Platformu kurar (engine Init/Open/Start ÇAĞRILMIŞ OLMALI).
 *
 * Dünya (BBox ±500 — binary physics AABB), perspektif kamera (near 0.1,
 * far 5000, viewWindow 1×1), ambient + directional ışık oluşturur ve
 * dünyaya ekler. Tekrar çağrı TRUE döner (idempotent).
 *
 * @param width  Raster genişliği (bilgi amaçlı — RCCCameraView raster
 *               alanları; GL3'te gerçek framebuffer boyutu).
 * @param height Raster yüksekliği.
 * @return TRUE kuruldu/zaten kuruldu; FALSE engine çalışmıyor veya
 *         tahsis hatası.
 */
RwBool RenderPlatform_Initialize(RwInt32 width, RwInt32 height);

/// Platform kurulu mu (tekil world/kamera).
RwBool RenderPlatform_IsInitialized(void);

/// Dünya (NULL değilse geçerli).
void *RenderPlatform_GetWorld(void);

/// Kamera (NULL değilse geçerli).
void *RenderPlatform_GetCamera(void);

/**
 * @brief Kare başlangıcı: clear (IMAGE|Z) → CameraBeginUpdate →
 * RenderPipeline_Reset + SetViewWindowFromCamera (kamera viewWindow +
 * frame LTM — RCCCameraView şeması).
 *
 * @param pClearCol Temizleme rengi (NULL → opak siyah).
 * @return TRUE kare başladı; FALSE platform kurulu değil.
 */
RwBool RenderPlatform_BeginFrame(const RwRGBA *pClearCol);

/// Kare bitişi: CameraEndUpdate → (frameBuffer varsa) ShowRaster.
void RenderPlatform_EndFrame(void);

/**
 * @brief Im2D test karesi — RenderPipeline matris yığını (ScaleXY +
 * TranslateXY) ile üst matris uygulanarak ekran-uzay dörtgen çizilir.
 *
 * Vertex düzeni: RwIm2DVertex (x,y,z,rhw,RGBA,u,v) — gl3::Im2DVertex
 * ile aynı bayt yerleşimi (GL3 geçişinde uyumlu). z = im2d::GetNearZ.
 *
 * @param cx,cy  Merkez (view-window birimi — üst matris öncesinde).
 * @param halfW,halfH Yarı boyut (view-window birimi).
 * @param argb   Çizim rengi (0xAARRGGBB).
 * @return TRUE çizildi; FALSE platform kurulu değil.
 */
RwBool RenderPlatform_RenderTestQuad(RwReal cx, RwReal cy,
                                     RwReal halfW, RwReal halfH,
                                     RwUInt32 argb);

/**
 * @brief Sahneyi yok eder (kamera/ışık/dünya destroy). Engine
 * lifecycle'i SAHİBİ DEĞİLDİR — çağırıcı (test/main) stop/close/term
 * eder. Tekrar çağrı güvenlidir.
 */
void RenderPlatform_Terminate(void);

/**
 * @brief Kamera yerleşimi (deneme/görsel doğrulama — Faz 4.6 görsel).
 *
 * Kamera frame matrisini orthonormal (at/right/up) kurar ve updateObjects
 * ile LTM'i senkronlar. Platform kurulu değilse no-op.
 *
 * @param position Kamera konumu (3 float).
 * @param lookAt   Hedef nokta (3 float — at vektörü buraya bakar).
 */
void RenderPlatform_SetCameraPose(const RwReal position[3],
                                  const RwReal lookAt[3]);

/**
 * @brief Clump'ı platform dünyasına ekler (render + world sahipliği
 * ATOMIC/CLUMP düzeyinde — clump destroy çağıranın görevi).
 *
 * Faz 4.6o: WorldGeometry_CreateClump çıktısı buraya bağlanır.
 *
 * @param pClump rw::Clump* (NULL → FALSE).
 * @return TRUE eklendi; FALSE platform kurulu değil veya NULL.
 */
RwBool RenderPlatform_AddClump(void *pClump);

/**
 * @brief Dünya render'ı — BeginFrame ile EndFrame ARASINDA çağrılır
 * (world->render(); NULL platform no-op).
 */
void RenderPlatform_RenderScene(void);

/**
 * @brief Im2D overlay geçişi başlat — render state'leri 2D'ye hazırlar
 *        (doku yok, Z test/write kapalı, vertex alpha + srcalpha blend).
 *        BeginFrame ile EndFrame ARASINDA, Im2D çiziminden ÖNCE çağrılır;
 *       3D sahne derinlik yazmışsa ZTEST Im2D'yi elerdi.
 */
void RenderPlatform_BeginOverlay2D(void);

/**
 * @brief Im2D overlay geçişi bitir —3D render state'lerini geri yükler.
 */
void RenderPlatform_EndOverlay2D(void);

/**
 * @brief Piksel koordinatlı dolu Im2D quad (ekran uzayı; CShowText deseni).
 *        x,y sol-üst köşe; w,h piksel; argb renk. kamera raster'ından
 *        ekran boyutunu alır. BeginOverlay2D sonrası çağrılır.
 */
void RenderPlatform_DrawOverlayQuad(RwReal x, RwReal y, RwReal w, RwReal h,
                                    RwUInt32 argb);

/**
 * @brief Bir sonraki EndFrame'de framebuffer'ı PNG olarak kaydettirir
 *        (swap öncesi; GL3). Tek seferlik — yazıldıktan sonra temizlenir.
 *        Headless/NULL platformda no-op. path=nullptr isteği iptal eder.
 */
void RenderPlatform_RequestScreenshot(const char *path);

/**
 * @brief Gömülü txd dokusu yükler (TxdDecode RGBA → librw Image→Raster→
 *        Texture). Aynı isim varsa eskisi destroy edilir. NULL'da no-op.
 * @return TRUE (yüklendi).
 */
RwBool RenderPlatform_UploadTexture(const char *name, RwInt32 width,
                                    RwInt32 height, const RwUInt8 *rgbaPixels,
                                    RwInt32 pitch);

/** @brief Yüklü doku var mı (isimle)? */
RwBool RenderPlatform_HasTexture(const char *name);

/**
 * @brief Dokulu Im2D quad (UV 0..1; beyaz tint). BeginOverlay2D sonrası
 *        çağrılır; TEXTURERASTER'ı kurup sonra sıfırlar. Doku yoksa çizmez.
 */
void RenderPlatform_DrawTexturedQuad(RwReal x, RwReal y, RwReal w, RwReal h,
                                     const char *texName);

/** @brief Tüm yüklenmiş dokuları destroy eder (menü teardown/terminate). */
void RenderPlatform_ClearTextures(void);

/**
 * @brief Seviye TexDictionary'yi kurar (0xe8 txd → librw dict + current).
 *        RwsPipeline_Load ÖNCESİ çağrılır — geometri malzemeleri stream
 *        anında isimle çözülür. Eski dict varsa destroy edilir (pipeline
 *        unload SONRASI çağır: malzemeler ham işaretçi tutar).
 * @return TRUE (dict kuruldu; en az bir doku).
 */
RwBool RenderPlatform_LoadWorldDictionary(const char *rwsPath);

/** @brief Dünya dict'ini destroy eder (pipeline unload sonrası/terminate). */
void RenderPlatform_ClearWorldDictionary(void);

#endif // CANNONCRUISE_RENDERPLATFORM_H

// ============================================================================
// CannonCruise - Render Platformu (RenderPlatform.cpp)
// Faz 4.6n: librw sahne kurulumu + kare döngüsü + Im2D test karesi.
// Binary karşılıkları (belgeli): sub_4189E0 MainLoop, FUN_0041C380 Idle,
// sub_41CEB0 Render (Begin/EndUpdate), FUN_004cc1f0 RenderPipeline_Reset,
// FUN_004cc020 SetViewWindowFromCamera.
//
// NULL platform: device no-op → headless smoke; GL3: gerçek çizim
// (ShowRaster frameBuffer nil ise atlanır — engine.cpp:360 null-safety).
// ============================================================================

#include "RenderPlatform.h"
#include "RenderPipeline.h"
#include "TxdDecode.h"

#include <rw.h>

#ifdef RW_GL3
// Ham GL (mipmap tamamlama için) — librw glad'i bu TU'da yok; framework
// linkli (GL3 build). NULL build'de derlenmez.
#include <OpenGL/gl3.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

rw::World *s_pWorld = nullptr;
rw::Camera *s_pCamera = nullptr;
rw::Light *s_pAmbient = nullptr;
rw::Light *s_pDirectional = nullptr;
rw::Frame *s_pCameraFrame = nullptr;
rw::Frame *s_pLightFrame = nullptr;
RwBool s_lightsAdded = FALSE; // addLight yapıldı mı (removeLight guard'ı)
RwInt32 s_width = 0;
RwInt32 s_height = 0;
char s_screenshotPath[512] = {0}; // EndFrame swap öncesi PNG yakalama

/// RCCCameraView + RenderPipeline_SetViewWindowFromCamera bağlantısı.
/// Kamera viewWindow (yarı boyut) ×2 + frame LTM (CCMatrix yerleşimi —
/// librw Matrix birebir aynı 64B) + raster boyutu (frameBuffer yoksa
/// platform width/height — bilgi amaçlı).
void ApplyCameraToPipeline(void) {
    RCCCameraView view;
    view.projFlag = s_pCamera->projection;
    view.rasterWidth = s_width;
    view.rasterHeight = s_height;
    view.viewWindowX = s_pCamera->viewWindow.x;
    view.viewWindowY = s_pCamera->viewWindow.y;
    view.pFrameMatrix =
        reinterpret_cast<const CCMatrix *>(s_pCameraFrame->getLTM());
    RenderPipeline_SetViewWindowFromCamera(&view);
}

} // namespace

RwBool RenderPlatform_Initialize(RwInt32 width, RwInt32 height) {
    if (RwEngineIsRunning() != TRUE) {
        std::fprintf(stderr, "RenderPlatform: engine not running\n");
        return FALSE;
    }
    if (s_pWorld != nullptr) {
        return TRUE; // idempotent
    }

    // Dünya — binary physics AABB ±500 (gravity/floor plane ile aynı
    // sınır; Faz 4.1 reverse notu).
    rw::BBox bbox;
    bbox.inf = rw::makeV3d(-500.0f, -500.0f, -500.0f);
    bbox.sup = rw::makeV3d(500.0f, 500.0f, 500.0f);
    s_pWorld = rw::World::create(&bbox);
    if (s_pWorld == nullptr) {
        std::fprintf(stderr, "RenderPlatform: World::create failed\n");
        return FALSE;
    }

    // Kamera — perspektif; librw default near/far (0.05/10) oyun için
    // çok dar → oyun değerleri (near 0.1, far 5000).
    s_pCamera = rw::Camera::create();
    s_pCameraFrame = rw::Frame::create();
    if (s_pCamera == nullptr || s_pCameraFrame == nullptr) {
        std::fprintf(stderr, "RenderPlatform: Camera/Frame create failed\n");
        RenderPlatform_Terminate();
        return FALSE;
    }
    s_pCamera->setFrame(s_pCameraFrame);
    s_pCamera->setNearPlane(0.1f);
    s_pCamera->setFarPlane(5000.0f);
    // viewWindow (yarı boyut) 1.0 — 0.6 denerken GL3 frustum culling
    // tüm atomics'i eledi (boş kare); culling çözülene kadar 1.0.
    const rw::V2d kViewWindow = rw::makeV2d(1.0f, 1.0f);
    s_pCamera->setViewWindow(&kViewWindow);
    s_pCamera->setProjection(rw::Camera::PERSPECTIVE);
#ifdef RW_GL3
    // GL3: gerçek framebuffer (skeleton CameraCreate deseni) — ShowRaster
    // bu raster'ı pencereye kopyalar; NULL platformda raster yok (no-op).
    s_pCamera->frameBuffer =
        rw::Raster::create(width, height, 0, rw::Raster::CAMERA);
    s_pCamera->zBuffer =
        rw::Raster::create(width, height, 0, rw::Raster::ZBUFFER);
    if (s_pCamera->frameBuffer == nullptr ||
        s_pCamera->zBuffer == nullptr) {
        std::fprintf(stderr, "RenderPlatform: camera raster create failed\n");
        RenderPlatform_Terminate();
        return FALSE;
    }
#endif
    s_pWorld->addCamera(s_pCamera);

    // Işıklar — ambient + directional (GL3'te gölgeleme için; NULL
    // platformda no-op). Yön: identity frame → -Z (RW directional
    // at vektörü).
    s_pAmbient = rw::Light::create(rw::Light::AMBIENT);
    s_pDirectional = rw::Light::create(rw::Light::DIRECTIONAL);
    if (s_pAmbient != nullptr && s_pDirectional != nullptr) {
        s_pAmbient->setColor(0.35f, 0.35f, 0.35f);
        s_pDirectional->setColor(0.85f, 0.85f, 0.85f);
        s_pLightFrame = rw::Frame::create();
        if (s_pLightFrame != nullptr) {
            s_pDirectional->setFrame(s_pLightFrame);
        }
        s_pWorld->addLight(s_pAmbient);
        s_pWorld->addLight(s_pDirectional);
        s_lightsAdded = TRUE;
    }

    s_width = width;
    s_height = height;
    return TRUE;
}

RwBool RenderPlatform_IsInitialized(void) {
    return (s_pWorld != nullptr && s_pCamera != nullptr) ? TRUE : FALSE;
}

void *RenderPlatform_GetWorld(void) { return s_pWorld; }

void *RenderPlatform_GetCamera(void) { return s_pCamera; }

RwBool RenderPlatform_BeginFrame(const RwRGBA *pClearCol) {
    if (RenderPlatform_IsInitialized() != TRUE) {
        return FALSE;
    }

    // Clear — IMAGE|Z (binary render geçişi; NULL platform no-op).
    static rw::RGBA s_defaultClear = {32, 48, 64, 255}; // açık deniz mavisi
    rw::RGBA *pCol = pClearCol
                         ? reinterpret_cast<rw::RGBA *>(const_cast<RwRGBA *>(pClearCol))
                         : &s_defaultClear;
    s_pCamera->clear(pCol, rw::Camera::CLEARIMAGE | rw::Camera::CLEARZ);

    // BeginUpdate — world sync + viewMatrix (cameraSync; NULL no-op).
    s_pCamera->beginUpdate();

    // RenderPipeline bağlamı — binary render geçişi FUN_004cc1f0 +
    // FUN_004cc020 sırası.
    RenderPipeline_Reset();
    ApplyCameraToPipeline();
    // Sis kapalı (uzak arazi kararıyor şüphesi — teşhis).
    rw::SetRenderState(rw::FOGENABLE, 0);
    return TRUE;
}

void RenderPlatform_EndFrame(void) {
    if (RenderPlatform_IsInitialized() != TRUE) {
        return;
    }
    s_pCamera->endUpdate();
    // Ekran görüntüsü isteği (swap öncesi) — GL3 framebuffer → PNG.
    if (s_screenshotPath[0] != '\0' && s_pCamera->frameBuffer != nullptr) {
#ifdef RW_GL3
        rw::Image *pShot = s_pCamera->frameBuffer->toImage();
        if (pShot != nullptr) {
            rw::writePNG(pShot, s_screenshotPath);
            pShot->destroy();
            std::printf("Screenshot: %s\n", s_screenshotPath);
        }
#endif
        s_screenshotPath[0] = '\0'; // tek seferlik
    }
    if (s_pCamera->frameBuffer != nullptr) {
        s_pCamera->showRaster(0);
    }
    // GL3: GLFW olayları (ESC/kapat) — main loop ShouldClose ile biter.
    RwPlatformPollEvents();
}

void RenderPlatform_RequestScreenshot(const char *path) {
    if (path == nullptr) {
        s_screenshotPath[0] = '\0';
        return;
    }
    std::snprintf(s_screenshotPath, sizeof(s_screenshotPath), "%s", path);
}

RwBool RenderPlatform_RenderTestQuad(RwReal cx, RwReal cy,
                                     RwReal halfW, RwReal halfH,
                                     RwUInt32 argb) {
    if (RenderPlatform_IsInitialized() != TRUE) {
        return FALSE;
    }

    // Üst matris — ScaleXY + TranslateXY (RenderPipeline yığını; FUN_
    // 004cc480 / FUN_004cc4d0). Köşe pozisyonları bu matrisle 2B afin
    // çarpılır (view-window uzayı → ekran uzayı köprüsü; GL3 pass'ında
    // tam yol BuildViewMatrix).
    RenderPipeline_MatrixIdentity();
    RenderPipeline_MatrixScaleXY(halfW * 2.0f, halfH * 2.0f);
    RenderPipeline_MatrixTranslateXY(cx, cy);

    const CCMatrix *pTop = RenderPipeline_GetTopMatrix();
    if (pTop == nullptr) {
        return FALSE;
    }

    const RwReal z = rw::im2d::GetNearZ();
    const RwReal rhw = 1.0f;
    const RwUInt8 r = static_cast<RwUInt8>((argb >> 16) & 0xFF);
    const RwUInt8 g = static_cast<RwUInt8>((argb >> 8) & 0xFF);
    const RwUInt8 b = static_cast<RwUInt8>(argb & 0xFF);
    const RwUInt8 a = static_cast<RwUInt8>((argb >> 24) & 0xFF);

    // Birim kare (-0.5..+0.5) — scale (halfW×2) + translate (cx,cy)
    // ile top matriste; transform: x' = right·x + up·y + pos.
    const RwReal kCorners[4][2] = {{-0.5f, -0.5f},
                                   {0.5f, -0.5f},
                                   {-0.5f, 0.5f},
                                   {0.5f, 0.5f}};
    RwIm2DVertex verts[4];
    for (int i = 0; i < 4; ++i) {
        const RwReal lx = kCorners[i][0];
        const RwReal ly = kCorners[i][1];
        verts[i].x = pTop->right[0] * lx + pTop->up[0] * ly + pTop->pos[0];
        verts[i].y = pTop->right[1] * lx + pTop->up[1] * ly + pTop->pos[1];
        verts[i].z = z;
        verts[i].rhw = rhw;
        verts[i].color.red = r;
        verts[i].color.green = g;
        verts[i].color.blue = b;
        verts[i].color.alpha = a;
        verts[i].u = (lx + 0.5f);
        verts[i].v = (ly + 0.5f);
    }

    // İki üçgen (TRILIST + indeks) — NULL platform no-op driver;
    // doğrudan rw::im2d (stub C API rwstubs no-op'una düşmez).
    RwUInt16 indices[6] = {0, 1, 2, 2, 1, 3};
    rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST, verts, 4,
                                     indices, 6);
    return TRUE;
}

void RenderPlatform_Terminate(void) {
    if (s_pWorld != nullptr) {
        if (s_pCamera != nullptr) {
            s_pWorld->removeCamera(s_pCamera);
#ifdef RW_GL3
            // Raster'lar kameradan önce serbest (skeleton CameraDestroy).
            if (s_pCamera->frameBuffer != nullptr) {
                s_pCamera->frameBuffer->destroy();
                s_pCamera->frameBuffer = nullptr;
            }
            if (s_pCamera->zBuffer != nullptr) {
                s_pCamera->zBuffer->destroy();
                s_pCamera->zBuffer = nullptr;
            }
#endif
            s_pCamera->destroy();
            s_pCamera = nullptr;
        }
        if (s_pAmbient != nullptr) {
            // Yalnızca eklenmiş ışık kaldırılır (eklenmemiş LLLink
            // removeLight'ta SEGV yapar — NOLIGHTS crash dersi).
            if (s_lightsAdded == TRUE) {
                s_pWorld->removeLight(s_pAmbient);
            }
            s_pAmbient->destroy();
            s_pAmbient = nullptr;
        }
        if (s_pDirectional != nullptr) {
            if (s_lightsAdded == TRUE) {
                s_pWorld->removeLight(s_pDirectional);
            }
            s_pDirectional->destroy();
            s_pDirectional = nullptr;
        }
        s_lightsAdded = FALSE;
        s_pWorld->destroy();
        s_pWorld = nullptr;
    }
    if (s_pCameraFrame != nullptr) {
        s_pCameraFrame->destroy();
        s_pCameraFrame = nullptr;
    }
    if (s_pLightFrame != nullptr) {
        s_pLightFrame->destroy();
        s_pLightFrame = nullptr;
    }
    RenderPlatform_ClearTextures();
    RenderPlatform_ClearWorldDictionary();
    s_width = 0;
    s_height = 0;
}

RwBool RenderPlatform_AddClump(void *pClump) {
    if (RenderPlatform_IsInitialized() != TRUE || pClump == nullptr) {
        return FALSE;
    }
    s_pWorld->addClump(static_cast<rw::Clump *>(pClump));
    return TRUE;
}

void RenderPlatform_RenderScene(void) {
    if (RenderPlatform_IsInitialized() != TRUE) {
        return;
    }
    s_pWorld->render();
}

// ---------------------------------------------------------------------------
// Im2D overlay — piksel koordinatlı2D (CShowText::Render deseni).
// 3D sahne derinlik yazdığı için ZTEST kapalı yoksa Im2D elenir.
// librw C++ API (rw::SetRenderState) — C stub RwRenderStateSet GL3'te yok.
// ---------------------------------------------------------------------------
void RenderPlatform_BeginOverlay2D(void) {
    rw::SetRenderStatePtr(rw::TEXTURERASTER, nullptr);
    rw::SetRenderState(rw::ZTESTENABLE, 0);
    rw::SetRenderState(rw::ZWRITEENABLE, 0);
    rw::SetRenderState(rw::VERTEXALPHA, 1);
    rw::SetRenderState(rw::SRCBLEND, rw::BLENDSRCALPHA);
    rw::SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
    // 3D sahneden kalan mipmap'li filtre dokulu Im2D'yi karartır
    // ("incomplete texture"); overlay'de mipmapsiz linear + clamp.
    rw::SetRenderState(rw::TEXTUREFILTER, rw::Texture::LINEAR);
    rw::SetRenderState(rw::TEXTUREADDRESS, rw::Texture::CLAMP);
}

void RenderPlatform_EndOverlay2D(void) {
    rw::SetRenderState(rw::VERTEXALPHA, 0);
    rw::SetRenderState(rw::ZTESTENABLE, 1);
    rw::SetRenderState(rw::ZWRITEENABLE, 1);
}

void RenderPlatform_DrawOverlayQuad(RwReal x, RwReal y, RwReal w, RwReal h,
                                    RwUInt32 argb) {
    if (RenderPlatform_IsInitialized() != TRUE) {
        return;
    }
    const RwReal z = rw::im2d::GetNearZ();
    const RwReal rhw = 1.0f;
    const RwUInt8 r = static_cast<RwUInt8>((argb >> 16) & 0xFF);
    const RwUInt8 g = static_cast<RwUInt8>((argb >> 8) & 0xFF);
    const RwUInt8 b = static_cast<RwUInt8>(argb & 0xFF);
    const RwUInt8 a = static_cast<RwUInt8>((argb >> 24) & 0xFF);

    // Piksel koordinatlı dörtgen (CShowText bg deseni): sol-üst, sol-alt,
    // sağ-üst, sağ-alt — TRILIST + indeks.
    const RwReal vx[4] = {x, x, x + w, x + w};
    const RwReal vy[4] = {y, y + h, y, y + h};
    RwIm2DVertex verts[4];
    for (int i = 0; i < 4; ++i) {
        verts[i].x = vx[i];
        verts[i].y = vy[i];
        verts[i].z = z;
        verts[i].rhw = rhw;
        verts[i].color.red = r;
        verts[i].color.green = g;
        verts[i].color.blue = b;
        verts[i].color.alpha = a;
        verts[i].u = 0.0f;
        verts[i].v = 0.0f;
    }
    RwUInt16 indices[6] = {0, 1, 2, 2, 1, 3};
    rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST, verts, 4,
                                     indices, 6);
}

// ---------------------------------------------------------------------------
// Gömülü menü dokuları (MAINMENU.RWS TexDictionary → librw Texture).
// TxdDecode pikseli zaten RGBA verir (TxdDecode.h) — swizzle YOK.
// ---------------------------------------------------------------------------
static std::vector<rw::Texture *> s_menuTextures;

static rw::Texture *FindMenuTexture(const char *name)
{
    if (name == nullptr) {
        return nullptr;
    }
    for (size_t i = 0; i < s_menuTextures.size(); ++i) {
        if (std::strcmp(s_menuTextures[i]->name, name) == 0) {
            return s_menuTextures[i];
        }
    }
    return nullptr;
}

// RGBA piksellerden librw dokusu (Image→Raster→Texture). NULL başarısızlık.
static rw::Texture *CreateTextureFromRGBA(const char *name, RwInt32 width,
                                          RwInt32 height,
                                          const RwUInt8 *rgbaPixels,
                                          RwInt32 pitch)
{
    if (name == nullptr || rgbaPixels == nullptr || width <= 0 ||
        height <= 0 || pitch < width * 4) {
        return nullptr;
    }
    rw::Image *img = rw::Image::create(width, height, 32);
    if (img == nullptr) {
        return nullptr;
    }
    img->allocate();
    if (img->pixels == nullptr) {
        img->destroy();
        return nullptr;
    }
    for (RwInt32 y = 0; y < height; ++y) {
        const RwUInt8 *src = rgbaPixels + (size_t)y * (size_t)pitch;
        RwUInt8 *dst = img->pixels + (size_t)y * (size_t)img->stride;
        for (RwInt32 x = 0; x < width; ++x) {
            dst[x * 4 + 0] = src[x * 4 + 0]; // R (TxdDecode zaten RGBA)
            dst[x * 4 + 1] = src[x * 4 + 1]; // G
            dst[x * 4 + 2] = src[x * 4 + 2]; // B
            dst[x * 4 + 3] = src[x * 4 + 3]; // A
        }
    }
    // Teşhis: CANNONCRUISE_REDTEX=1 → tüm dokular kırmızı (örnekleme testi).
    if (std::getenv("CANNONCRUISE_REDTEX") != nullptr) {
        for (RwInt32 y = 0; y < height; ++y) {
            RwUInt8 *dst = img->pixels + (size_t)y * (size_t)img->stride;
            for (RwInt32 x = 0; x < width; ++x) {
                dst[x * 4 + 0] = 255;
                dst[x * 4 + 1] = 16;
                dst[x * 4 + 2] = 16;
                dst[x * 4 + 3] = 255;
            }
        }
    }
    rw::Raster *raster = rw::Raster::createFromImage(img, 0);
    img->destroy();
    if (raster == nullptr) {
        return nullptr;
    }
    rw::Texture *tex = rw::Texture::create(raster);
    if (tex == nullptr) {
        raster->destroy();
        return nullptr;
    }
    std::snprintf(tex->name, sizeof(tex->name), "%s", name);
#ifdef RW_GL3
    // Mipmap tamamlama: librw Gl3Raster numLevels/autogenMipmap init
    // etmez (çöp değer MIP filtresi → "incomplete texture" = siyah).
    // Gerçek mip'ler üretilip MAX_LEVEL kısılırsa HER dal complete olur.
    {
        void *prevTex = rw::GetRenderStatePtr(rw::TEXTURERASTER);
        rw::SetRenderStatePtr(rw::TEXTURERASTER, raster);
        // Mipmap üret + MAX_LEVEL kıs: HER dal complete olur (garbage
        // numLevels/autogenMipmap → MIP filtresi siyah yapıyordu).
        int levels = 1;
        {
            int span = (width > height) ? width : height;
            while (span > 1) {
                span /= 2;
                ++levels;
            }
        }
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels - 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Teşhis: CANNONCRUISE_NEAREST=1 → NEAREST örnekleme.
        if (std::getenv("CANNONCRUISE_NEAREST") != nullptr) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                            GL_NEAREST);
        }
        rw::SetRenderStatePtr(rw::TEXTURERASTER, prevTex);
    }
#endif
    return tex;
}

RwBool RenderPlatform_UploadTexture(const char *name, RwInt32 width,
                                    RwInt32 height, const RwUInt8 *rgbaPixels,
                                    RwInt32 pitch)
{
    if (name == nullptr || rgbaPixels == nullptr || width <= 0 ||
        height <= 0 || pitch < width * 4) {
        return FALSE;
    }
    rw::Texture *old = FindMenuTexture(name);
    if (old != nullptr) {
        for (size_t i = 0; i < s_menuTextures.size(); ++i) {
            if (s_menuTextures[i] == old) {
                s_menuTextures.erase(s_menuTextures.begin() +
                                     (ptrdiff_t)i);
                break;
            }
        }
        old->destroy();
    }
    rw::Texture *tex =
        CreateTextureFromRGBA(name, width, height, rgbaPixels, pitch);
    if (tex == nullptr) {
        return FALSE;
    }
    // 2D UI: mipmap yok, linear + clamp (asset filterFlags göz ardı —
    // sehpa üstü eksik mip seviyesi "incomplete texture" = siyah yapar).
    tex->setFilter(rw::Texture::LINEAR);
    tex->setAddressU(rw::Texture::CLAMP);
    tex->setAddressV(rw::Texture::CLAMP);
    s_menuTextures.push_back(tex);
    return TRUE;
}

RwBool RenderPlatform_HasTexture(const char *name)
{
    return (FindMenuTexture(name) != nullptr) ? TRUE : FALSE;
}

void RenderPlatform_DrawTexturedQuad(RwReal x, RwReal y, RwReal w, RwReal h,
                                     const char *texName)
{
    if (RenderPlatform_IsInitialized() != TRUE) {
        return;
    }
    rw::Texture *tex = FindMenuTexture(texName);
    if (tex == nullptr || tex->raster == nullptr) {
        return;
    }
    rw::SetRenderStatePtr(rw::TEXTURERASTER, tex->raster);
    const RwReal z = rw::im2d::GetNearZ();
    RwIm2DVertex verts[4];
    const RwReal vx[4] = {x, x, x + w, x + w};
    const RwReal vy[4] = {y, y + h, y, y + h};
    const RwReal vu[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    const RwReal vv[4] = {0.0f, 1.0f, 0.0f, 1.0f};
    for (int i = 0; i < 4; ++i) {
        verts[i].x = vx[i];
        verts[i].y = vy[i];
        verts[i].z = z;
        verts[i].rhw = 1.0f;
        verts[i].color.red = 255;
        verts[i].color.green = 255;
        verts[i].color.blue = 255;
        verts[i].color.alpha = 255;
        verts[i].u = vu[i];
        verts[i].v = vv[i];
    }
    RwUInt16 indices[6] = {0, 1, 2, 2, 1, 3};
    rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST, verts, 4,
                                     indices, 6);
    rw::SetRenderStatePtr(rw::TEXTURERASTER, nullptr);
}

void RenderPlatform_ClearTextures(void)
{
    for (size_t i = 0; i < s_menuTextures.size(); ++i) {
        s_menuTextures[i]->destroy();
    }
    s_menuTextures.clear();
}

// ---------------------------------------------------------------------------
// Seviye TexDictionary (3D geometri malzemeleri stream anında isimle çözer).
// Ömür: pipeline unload SONRASI değiştir/yoket (malzemeler ham tutar).
// ---------------------------------------------------------------------------
static rw::TexDictionary *s_worldDict = nullptr;

RwBool RenderPlatform_LoadWorldDictionary(const char *rwsPath)
{
    if (rwsPath == nullptr) {
        return FALSE;
    }
    FILE *f = std::fopen(rwsPath, "rb");
    if (f == nullptr) {
        return FALSE;
    }
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    RwBool any = FALSE;
    rw::TexDictionary *dict = nullptr;
    if (size > 12 + 8) {
        RwUInt8 *buf = static_cast<RwUInt8 *>(std::malloc((size_t)size));
        if (buf != nullptr &&
            std::fread(buf, 1, (size_t)size, f) == (size_t)size) {
            // Seviye doku paketi: ilk 0x716 chunk'u (özel kayıt formatı;
            // menü d3d8-IMAGE'inden farklı).
            RwUInt32 c0len = 0;
            std::memcpy(&c0len, buf + 4, 4);
            RwUInt32 scanLen = (RwUInt32)size;
            if (c0len > 12 && 12 + c0len <= (RwUInt32)size) {
                scanLen = 12 + c0len;
            }
            RwTxdImage *pImage = static_cast<RwTxdImage *>(
                std::malloc(sizeof(RwTxdImage)));
            if (pImage != nullptr) {
                if (RwTxdDecodeLevelPack(buf, scanLen, pImage) == TRUE &&
                    pImage->count > 0) {
                    std::printf("WorldDict: %s: %u textures\n", rwsPath,
                                pImage->count);
                    dict = rw::TexDictionary::create();
                    if (dict != nullptr) {
                        for (RwUInt32 i = 0; i < pImage->count; ++i) {
                            const RwTxdTexture *t = &pImage->textures[i];
                            if (t->pPixels == nullptr || t->width == 0 ||
                                t->height == 0) {
                                continue;
                            }
                            rw::Texture *tex = CreateTextureFromRGBA(
                                t->name, (RwInt32)t->width,
                                (RwInt32)t->height, t->pPixels,
                                (RwInt32)t->pitch);
                            if (tex == nullptr) {
                                continue;
                            }
                            // Arazi döşemesi: wrap + linear (mipmapsiz).
                            tex->setFilter(rw::Texture::LINEAR);
                            tex->setAddressU(rw::Texture::WRAP);
                            tex->setAddressV(rw::Texture::WRAP);
                            dict->add(tex);
                            any = TRUE;
                        }
                    }
                }
                std::free(pImage);
            }
            std::free(buf);
        }
    }
    std::fclose(f);
    if (any == FALSE) {
        if (dict != nullptr) {
            dict->destroy();
        }
        return FALSE;
    }
    if (s_worldDict != nullptr) {
        s_worldDict->destroy();
    }
    s_worldDict = dict;
    rw::TexDictionary::setCurrent(s_worldDict);
#ifdef RW_GL3
    // Teşhis: CANNONCRUISE_DUMPTEX=ad → GL içeriğini PPM yazar.
    {
        const char *dumpName = std::getenv("CANNONCRUISE_DUMPTEX");
        if (dumpName != nullptr) {
            rw::Texture *dt = s_worldDict->find(dumpName);
            if (dt != nullptr && dt->raster != nullptr) {
                void *prevTex =
                    rw::GetRenderStatePtr(rw::TEXTURERASTER);
                rw::SetRenderStatePtr(rw::TEXTURERASTER, dt->raster);
                const int dw = dt->raster->width;
                const int dh = dt->raster->height;
                RwUInt8 *dpix = static_cast<RwUInt8 *>(
                    std::malloc((size_t)dw * dh * 4));
                if (dpix != nullptr) {
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA,
                                  GL_UNSIGNED_BYTE, dpix);
                    FILE *o = std::fopen("/tmp/gltex.ppm", "wb");
                    if (o != nullptr) {
                        std::fprintf(o, "P6\n%d %d\n255\n", dw, dh);
                        for (int yy = dh - 1; yy >= 0; --yy) {
                            for (int xx = 0; xx < dw; ++xx) {
                                RwUInt8 *p =
                                    dpix + ((size_t)yy * dw + xx) * 4;
                                std::fputc(p[0], o);
                                std::fputc(p[1], o);
                                std::fputc(p[2], o);
                            }
                        }
                        std::fclose(o);
                        std::printf("DumpTex: %s %dx%d -> /tmp/gltex.ppm\n",
                                    dumpName, dw, dh);
                    }
                    std::free(dpix);
                }
                rw::SetRenderStatePtr(rw::TEXTURERASTER, prevTex);
            } else {
                std::printf("DumpTex: '%s' not found\n", dumpName);
            }
        }
    }
    // Filtre durumu geri okuma (MIN/MAG/MAX) — DUMPTEX'ten bağımsız.
    {
        const char *qName = std::getenv("CANNONCRUISE_TEXQUERY");
        if (qName != nullptr && s_worldDict != nullptr) {
            rw::Texture *qt = s_worldDict->find(qName);
            void *pq = rw::GetRenderStatePtr(rw::TEXTURERASTER);
            rw::SetRenderStatePtr(rw::TEXTURERASTER,
                                  qt != nullptr ? qt->raster : nullptr);
            GLint mn = 0, mg = 0, mx = 0;
            glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                &mn);
            glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                                &mg);
            glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,
                                &mx);
            std::printf("TexQuery %s: MIN=0x%x MAG=0x%x MAX=%d\n", qName,
                        mn, mg, mx);
            rw::SetRenderStatePtr(rw::TEXTURERASTER, pq);
        }
    }
#endif
    return TRUE;
}

void RenderPlatform_ClearWorldDictionary(void)
{
    if (s_worldDict != nullptr) {
        s_worldDict->destroy();
        s_worldDict = nullptr;
    }
}

void RenderPlatform_SetCameraPose(const RwReal position[3],
                                  const RwReal lookAt[3]) {
    if (RenderPlatform_IsInitialized() != TRUE) {
        return;
    }
    rw::Matrix *pMat = &s_pCameraFrame->matrix;

    // at = normalize(lookAt - pos); right = normalize(cross(worldUp, at));
    // up = cross(at, right) — orthonormal RW frame (CCMatrix yerleşimi).
    rw::V3d at = rw::makeV3d(lookAt[0] - position[0],
                             lookAt[1] - position[1],
                             lookAt[2] - position[2]);
    const float lenAt = rw::length(at);
    if (lenAt > 1e-6f) {
        at = rw::normalize(at);
    } else {
        at = rw::makeV3d(0.0f, 0.0f, -1.0f);
    }
    rw::V3d worldUp = rw::makeV3d(0.0f, 1.0f, 0.0f);
    rw::V3d right = rw::cross(worldUp, at);
    const float lenRight = rw::length(right);
    if (lenRight > 1e-6f) {
        right = rw::normalize(right);
    } else {
        // at ∥ worldUp — x ekseni baz alınır.
        right = rw::makeV3d(1.0f, 0.0f, 0.0f);
    }
    const rw::V3d up = rw::cross(at, right);

    pMat->right = right;
    pMat->up = up;
    pMat->at = at;
    pMat->pos = rw::makeV3d(position[0], position[1], position[2]);
    s_pCameraFrame->updateObjects();
}

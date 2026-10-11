// ============================================================================
// CannonCruise - RenderWare SDK Stub Implementations (rwstubs.cpp)
//
// Bu dosya, statik kütüphanenin link sırasında ihtiyaç duyduğu tüm RenderWare
// 3.6 dış sembollerini sağlar (Rp*/Rw*/Rt* + Im3D + bellek).
//
// Durum: LINK STUB — fonksiyonlar derleyici/çalışma zamanı hatalarını önlemek
//        için no-op veya güvenli varsayılan değer döndürür. Gerçek RenderWare
//        SDK kütüphaneleriyle değiştirilmesi planlanmıştır (Faz 4.5 → 4.4b).
//
// Eksik sembol listesi `nm -u libCannonCruiseLib.a` ile doğrulanmıştır.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include <rphanim.h>
#include <rpskin.h>
#include <rpcollision.h>
#include <rtanim.h>
#include <rt2danim.h>
#include <cstdlib>
#include <cstring>

// ---------------------------------------------------------------------------
// Bellek (RwMalloc ailesi — Ocean_W32 dahil oyun kodu bunları çağırır)
// ---------------------------------------------------------------------------
void* RwMalloc(size_t size, RwUInt32 hint)
{
    (void)hint;
    return malloc(size);
}

void RwFree(void* mem)
{
    free(mem);
}

void* RwRealloc(void* mem, size_t size, RwUInt32 hint)
{
    (void)hint;
    return realloc(mem, size);
}

// ---------------------------------------------------------------------------
// Render State / Im2D / Im3D
// ---------------------------------------------------------------------------
// Birim testlerin çizim çağrısı sayısını gözlemlemesi için sayaçlar
// (FontSystem_DrawText batch flush doğrulaması).
static RwUInt32 s_im2DPrimitiveCount = 0;
static RwUInt32 s_im2DIndexedCount = 0;

RwUInt32 RwStub_GetIm2DRenderCount(void)
{
    return s_im2DPrimitiveCount + s_im2DIndexedCount;
}

RwUInt32 RwStub_GetIm2DIndexedCount(void)
{
    return s_im2DIndexedCount;
}

void RwStub_ResetIm2DRenderCount(void)
{
    s_im2DPrimitiveCount = 0;
    s_im2DIndexedCount = 0;
}

RwBool RwRenderStateSet(RwUInt32 state, void* value)
{
    (void)state; (void)value;
    return TRUE;
}

void* RwRenderStateGet(RwUInt32 state)
{
    (void)state;
    return 0;
}

RwBool RwIm2DRenderPrimitive(RwUInt32 primType, const RwIm2DVertex* vertices, RwInt32 numVertices)
{
    (void)primType; (void)vertices; (void)numVertices;
    ++s_im2DPrimitiveCount;
    return TRUE;
}

// Font çizim yolu (FontSystem_DrawText, sub_4CCC40 karşılığı) indeksli
// üçgen listesi gönderir (binary: sub_518FC0 + sub_518F90).
RwBool RwIm2DRenderIndexedPrimitive(RwUInt32 primType, const RwUInt16* indices, RwInt32 numIndices)
{
    (void)primType; (void)indices; (void)numIndices;
    ++s_im2DIndexedCount;
    return TRUE;
}

RwBool RwIm3DTransform(RwIm3DVertex* vertices, RwUInt32 numVertices,
                       const RwMatrix* ltm, RwUInt32 flags)
{
    (void)vertices; (void)numVertices; (void)ltm; (void)flags;
    return TRUE;
}

void RwIm3DEnd(void)
{
}

RwBool RwIm3DRenderPrimitive(RwUInt32 primType)
{
    (void)primType;
    return TRUE;
}

RwBool RwIm3DRenderIndexedPrimitive(RwUInt32 primType, RwUInt16* indices, RwInt32 numIndices)
{
    (void)primType; (void)indices; (void)numIndices;
    return TRUE;
}

// ---------------------------------------------------------------------------
// Matris
// ---------------------------------------------------------------------------
void RwMatrixSetIdentity(RwMatrix* matrix)
{
    if (!matrix) return;
    memset(matrix, 0, sizeof(RwMatrix));
    matrix->right.x = 1.0f;
    matrix->up.y    = 1.0f;
    matrix->at.z    = 1.0f;
}

void RwMatrixUpdate(RwMatrix* matrix)
{
    (void)matrix;
}

// ---------------------------------------------------------------------------
// Frame
// ---------------------------------------------------------------------------
RwFrame* RwFrameCreate(void)
{
    return (RwFrame*)calloc(1, sizeof(RwFrame));
}

RwBool RwFrameDestroy(RwFrame* frame)
{
    if (!frame) return FALSE;
    free(frame);
    return TRUE;
}

RwMatrix* RwFrameGetMatrix(RwFrame* frame)
{
    return frame ? &frame->modelling : 0;
}

RwMatrix* RwFrameGetLTM(RwFrame* frame)
{
    return frame ? &frame->ltm : 0;
}

RwFrame* RwFrameTranslate(RwFrame* frame, const RwV3d* v, RwOpCombineType combineOp)
{
    (void)v; (void)combineOp;
    return frame;
}

RwFrame* RwFrameRotate(RwFrame* frame, const RwV3d* axis, RwReal angle, RwOpCombineType combineOp)
{
    (void)axis; (void)angle; (void)combineOp;
    return frame;
}

RwFrame* RwFrameTransform(RwFrame* frame, const RwMatrix* matrix, RwOpCombineType combineOp)
{
    (void)matrix; (void)combineOp;
    return frame;
}

RwFrame* RwFrameUpdateObjects(RwFrame* frame)
{
    return frame;
}

RwFrame* RwFrameAddChild(RwFrame* parent, RwFrame* child)
{
    (void)child;
    return parent;
}

RwFrame* RwFrameForAllChildren(RwFrame* frame, RwFrameCallBack callBack, void* data)
{
    (void)callBack; (void)data;
    return frame;
}

// ---------------------------------------------------------------------------
// Raster
// ---------------------------------------------------------------------------
RwRaster* RwRasterCreate(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags)
{
    (void)width; (void)height; (void)depth; (void)flags;
    return 0; // Opaque tip — gerçek SDK'da heap tahsisi yapar
}

RwBool RwRasterDestroy(RwRaster* raster)
{
    (void)raster;
    return TRUE;
}

RwInt32 RwRasterGetWidth(const RwRaster* raster)
{
    (void)raster;
    return 0;
}

RwInt32 RwRasterGetHeight(const RwRaster* raster)
{
    (void)raster;
    return 0;
}

// ---------------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------------
RwCamera* RwCameraCreate(void)
{
    return 0; // Opaque tip
}

RwBool RwCameraDestroy(RwCamera* camera)
{
    (void)camera;
    return TRUE;
}

RwFrame* RwCameraGetFrame(const RwCamera* camera)
{
    (void)camera;
    return 0;
}

RwCamera* RwCameraSetFrame(RwCamera* camera, RwFrame* frame)
{
    (void)frame;
    return camera;
}

RwCamera* RwCameraSetRaster(RwCamera* camera, RwRaster* raster)
{
    (void)raster;
    return camera;
}

RwCamera* RwCameraSetZRaster(RwCamera* camera, RwRaster* zRaster)
{
    (void)zRaster;
    return camera;
}

RwCamera* RwCameraSetNearClipPlane(RwCamera* camera, RwReal nearClip)
{
    (void)nearClip;
    return camera;
}

RwCamera* RwCameraSetFarClipPlane(RwCamera* camera, RwReal farClip)
{
    (void)farClip;
    return camera;
}

RwCamera* RwCameraSetViewWindow(RwCamera* camera, const RwV2d* viewWindow)
{
    (void)viewWindow;
    return camera;
}

RwCamera* RwCameraClear(RwCamera* camera, RwRGBA* colour, RwInt32 clearMode)
{
    (void)colour; (void)clearMode;
    return camera;
}

RwCamera* RwCameraBeginUpdate(RwCamera* camera)
{
    return camera;
}

RwCamera* RwCameraEndUpdate(RwCamera* camera)
{
    return camera;
}

RwCamera* RwCameraShowRaster(RwCamera* camera, void* pDev, RwUInt32 flags)
{
    (void)pDev; (void)flags;
    return camera;
}

RwCamera* RwCameraGetCurrentCamera(void)
{
    return 0;
}

RwRaster* RwCameraGetRaster(RwCamera* camera)
{
    (void)camera;
    return 0;
}

RwRaster* RwCameraGetZRaster(RwCamera* camera)
{
    (void)camera;
    return 0;
}

RwReal RwCameraGetNearClipPlane(const RwCamera* camera)
{
    (void)camera;
    return 0.0f;
}

RwReal RwCameraGetFarClipPlane(const RwCamera* camera)
{
    (void)camera;
    return 0.0f;
}

// ---------------------------------------------------------------------------
// Engine lifecycle — GERÇEK implementasyon rw_librw.cpp'de (librw:
// rw::Engine::init/open/start/stop/close/term). Buradan taşındı (Faz 4.6d).
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Engine alt sistem / video modu (FUN_0050b060 komut dağıtıcı ailesi stub'ları)
// CameraInit (FUN_0041af10) bu sorgularla en iyi modu puanlar; test için
// kasıtlı olarak birden çok mod sunulur.
// ---------------------------------------------------------------------------
static RwInt32 s_activeSubSystem = 0;
static RwInt32 s_activeVideoMode = 0;

static const RwSubSystemInfo s_subSystems[] =
{
    { "D3D9 Hal Device" }
};

static const RwVideoMode s_videoModes[] =
{
    { 640,  480,  16, 0, 0, 0 },
    { 640,  480,  32, 0, 0, 0 },
    { 800,  600,  32, 0, 0, 0 },
    { 1024, 768,  32, 0, 0, 0 }
};

RwInt32 RwEngineGetNumSubSystems(void)
{
    return (RwInt32)(sizeof(s_subSystems) / sizeof(s_subSystems[0]));
}

RwBool RwEngineSetSubSystem(RwInt32 subSystemIndex)
{
    if (subSystemIndex < 0 || subSystemIndex >= RwEngineGetNumSubSystems())
        return FALSE;
    s_activeSubSystem = subSystemIndex;
    return TRUE;
}

RwBool RwEngineGetSubSystemInfo(RwSubSystemInfo* info, RwInt32 subSystemIndex)
{
    if (!info || subSystemIndex < 0 || subSystemIndex >= RwEngineGetNumSubSystems())
        return FALSE;
    *info = s_subSystems[subSystemIndex];
    return TRUE;
}

RwInt32 RwEngineGetNumVideoModes(void)
{
    return (RwInt32)(sizeof(s_videoModes) / sizeof(s_videoModes[0]));
}

RwBool RwEngineGetVideoModeInfo(RwVideoMode* videoMode, RwInt32 videoModeIndex)
{
    if (!videoMode || videoModeIndex < 0 || videoModeIndex >= RwEngineGetNumVideoModes())
        return FALSE;
    *videoMode = s_videoModes[videoModeIndex];
    return TRUE;
}

RwBool RwEngineSetVideoMode(RwInt32 videoModeIndex)
{
    if (videoModeIndex < 0 || videoModeIndex >= RwEngineGetNumVideoModes())
        return FALSE;
    s_activeVideoMode = videoModeIndex;
    return TRUE;
}

// ---------------------------------------------------------------------------
// Stream — GERÇEK implementasyon rw_librw.cpp'de (librw rw::StreamFile +
// rw::findChunk/readChunkHeaderInfo). RwStreamRead/Write da shim'de
// tanımlı (Faz 4.6d).
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Texture (rwcore.h: RwTextureRead/Destroy/SetRaster/GetRaster)
// Font sistemi (FontSystem.cpp, sub_5154D0) bu API'yi çağırır. Gerçek dosya
// yüklemesi olmadığından RwTextureRead NULL döndürür — binary'de doku
// bulunamazsa font yüklemesi de başarısız olur (aynı davranış).
// ---------------------------------------------------------------------------
struct RwTexture {
    RwRaster* raster;
    RwChar    name[32];
    RwChar    mask[32];
};

RwTexture* RwTextureRead(const RwChar* name, const RwChar* maskName)
{
    (void)name;
    (void)maskName;
    return 0; // Opaque tip — gerçek SDK'da dosya/txd yükler
}

RwBool RwTextureDestroy(RwTexture* texture)
{
    (void)texture;
    return TRUE;
}

RwTexture* RwTextureSetRaster(RwTexture* texture, RwRaster* raster)
{
    if (texture) {
        texture->raster = raster;
    }
    return texture;
}

RwRaster* RwTextureGetRaster(const RwTexture* texture)
{
    return texture ? texture->raster : 0;
}

// ---------------------------------------------------------------------------
// Texture Dictionary
// ---------------------------------------------------------------------------
RwBool RwTexDictionaryDestroy(RwTexDictionary* dict)
{
    (void)dict;
    return TRUE;
}

RwTexDictionary* RwTexDictionarySetCurrent(RwTexDictionary* dict)
{
    (void)dict;
    return 0;
}

// RwTexDictionaryStreamRead — GERÇEK implementasyon rw_librw.cpp'de
// (librw rw::TexDictionary::streamRead; Faz 4.6d).

// ---------------------------------------------------------------------------
// Atomic / Geometry (rpworld)
// ---------------------------------------------------------------------------
RwBool RpAtomicDestroy(RpAtomic* atomic)
{
    (void)atomic;
    return TRUE;
}

RpGeometry* RpAtomicGetGeometry(const RpAtomic* atomic)
{
    (void)atomic;
    return 0;
}

RpAtomic* RpAtomicSetFlags(RpAtomic* atomic, RwUInt32 flags)
{
    (void)flags;
    return atomic;
}

RpAtomic* RpAtomicSetFrame(RpAtomic* atomic, RwFrame* frame)
{
    (void)frame;
    return atomic;
}

// ---------------------------------------------------------------------------
// Clump (rpworld)
// ---------------------------------------------------------------------------
RwBool RpClumpDestroy(RpClump* clump)
{
    (void)clump;
    return TRUE;
}

RpClump* RpClumpClone(RpClump* clump)
{
    (void)clump;
    return 0;
}

void RpClumpRender(RpClump* clump)
{
    (void)clump;
}

RpClump* RpClumpForAllAtomics(RpClump* clump, RpAtomicCallBack callback, void* data)
{
    (void)callback; (void)data;
    return clump;
}

RwFrame* RpClumpGetFrame(RpClump* clump)
{
    (void)clump;
    return 0;
}

// RpClumpStreamRead — GERÇEK implementasyon rw_librw.cpp'de
// (librw rw::rp::Clump::streamRead; Faz 4.6d).

// CollisionTrigger (phantom AABB) için eklendi — librw'de gerçek veri
const RwSphere* RpClumpGetBoundingSphere(const RpClump* clump)
{
    (void)clump;
    return 0;
}

// ---------------------------------------------------------------------------
// Geometry (rpworld)
// ---------------------------------------------------------------------------
const RwSphere* RpGeometryGetBoundingSphere(const RpGeometry* geometry)
{
    (void)geometry;
    return 0;
}

// CollisionGeometry (Clump→şekil) için eklendi — librw entegrasyonunda
// gerçek vertex/triangle verisi döner
RwInt32 RpGeometryGetNumVertices(const RpGeometry* geometry)
{
    (void)geometry;
    return 0;
}

RwInt32 RpGeometryGetNumTriangles(const RpGeometry* geometry)
{
    (void)geometry;
    return 0;
}

RwV3d* RpGeometryGetVertices(RpGeometry* geometry)
{
    (void)geometry;
    return 0;
}

RwUInt16* RpGeometryGetTriangleIndices(RpGeometry* geometry)
{
    (void)geometry;
    return 0;
}

RwUInt32 RpGeometryGetFlags(const RpGeometry* geometry)
{
    (void)geometry;
    return 0;
}

// ---------------------------------------------------------------------------
// World (rpworld)
// ---------------------------------------------------------------------------
RpWorld* RpWorldCreate(RwBBox* boundingBox)
{
    (void)boundingBox;
    return 0;
}

RwBool RpWorldDestroy(RpWorld* world)
{
    (void)world;
    return TRUE;
}

RpWorld* RpWorldAddClump(RpWorld* world, RpClump* clump)
{
    (void)clump;
    return world;
}

RpWorld* RpWorldRemoveClump(RpWorld* world, RpClump* clump)
{
    (void)clump;
    return world;
}

void RpWorldRender(RpWorld* world)
{
    (void)world;
}

RpWorld* RpWorldStreamRead(RwStream* stream)
{
    // librw'de rp::World streamRead YOK (BSP world) — kapı 4 riski
    // belgeli; world streamleri henüz işlenemiyor (Faz 4.6d).
    (void)stream;
    return 0;
}

// ---------------------------------------------------------------------------
// Material / Plugin Attach
// ---------------------------------------------------------------------------
RwInt32 RpMaterialRegisterPlugin(RwInt32 size, RwUInt32 pluginID,
                                 RwPluginObjectConstructor constructCB,
                                 RwPluginObjectDestructor destructCB,
                                 RwPluginObjectCopy copyCB)
{
    (void)size; (void)pluginID; (void)constructCB; (void)destructCB; (void)copyCB;
    return -1; // Gerçek SDK'da eklenti ofseti döndürür
}

RwBool RpWorldPluginAttach(void)     { return TRUE; }
RwBool RpCollisionPluginAttach(void) { return TRUE; }
RwBool RpSkinPluginAttach(void)      { return TRUE; }
RwBool RpHAnimPluginAttach(void)     { return TRUE; }

// ---------------------------------------------------------------------------
// HAnim (rphanim)
// ---------------------------------------------------------------------------
void RpHAnimHierarchyUpdateMatrices(RpHAnimHierarchy* hierarchy)
{
    (void)hierarchy;
}

void RpHAnimHierarchyAddAnimTime(RpHAnimHierarchy* hierarchy, RwReal time)
{
    (void)hierarchy; (void)time;
}

void RpHAnimHierarchySetCurrentAnim(RpHAnimHierarchy* hierarchy, RtAnimAnimation* anim)
{
    (void)hierarchy; (void)anim;
}

void RpHAnimHierarchyBlendHierarchy(RpHAnimHierarchy* dest, RpHAnimHierarchy* src, RwReal alpha)
{
    (void)dest; (void)src; (void)alpha;
}

// ---------------------------------------------------------------------------
// Skin (rpworld + rphanim)
// ---------------------------------------------------------------------------
RpSkin* RpSkinAtomicGetSkin(const RpAtomic* atomic)
{
    (void)atomic;
    return 0;
}

RpHAnimHierarchy* RpSkinAtomicGetHAnimHierarchy(const RpAtomic* atomic)
{
    (void)atomic;
    return 0;
}

RpAtomic* RpSkinAtomicSetHAnimHierarchy(RpAtomic* atomic, RpHAnimHierarchy* hierarchy)
{
    (void)hierarchy;
    return atomic;
}

RpHAnimHierarchy* RpSkinSafeAtomicGetHAnimHierarchy(RpClump* clump)
{
    (void)clump;
    return 0;
}

void RpSkinSafeClumpSetHAnimHierarchy(RpClump* clump, RpHAnimHierarchy* hierarchy)
{
    (void)clump; (void)hierarchy;
}

// ---------------------------------------------------------------------------
// Anim (rtanim / rtcmpkey)
// ---------------------------------------------------------------------------
RwBool RtAnimInitialize(void)
{
    return TRUE;
}

RwBool RtAnimRegisterInterpolationScheme(void* scheme)
{
    (void)scheme;
    return TRUE;
}

void* RtCompressedKeyFrameGetCustomKeyFrameScheme(void)
{
    return 0;
}

// ---------------------------------------------------------------------------
// 2D Maestro (rt2danim) — gerçek implementasyon: Rt2dMaestro.cpp (4.6r)
// ---------------------------------------------------------------------------

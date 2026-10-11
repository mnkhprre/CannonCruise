#ifndef RWCORE_H
#define RWCORE_H

// ============================================================================
// RenderWare Graphics SDK 3.x - Core Definitions Stub (rwcore.h)
// Minimal types & signatures for CannonCruise build & IDE analysis.
// ============================================================================

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Base scalar types
typedef float          RwReal;
typedef int32_t        RwInt32;
typedef uint32_t       RwUInt32;
typedef int16_t        RwInt16;
typedef uint16_t       RwUInt16;
typedef char           RwChar;
typedef int8_t         RwInt8;
typedef uint8_t        RwUInt8;
typedef int32_t        RwBool;

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// Operation combine types
enum RwOpCombineType
{
    rwCOMBINEREPLACE = 0,
    rwCOMBINEPRECONCAT,
    rwCOMBINEPOSTCONCAT
};
typedef enum RwOpCombineType RwOpCombineType;

// Vector & Math Structures
struct RwV2d
{
    RwReal x;
    RwReal y;
};

struct RwV3d
{
    RwReal x;
    RwReal y;
    RwReal z;
};

struct RwTexCoords
{
    RwReal u;
    RwReal v;
};

struct RwRGBA
{
    RwUInt8 red;
    RwUInt8 green;
    RwUInt8 blue;
    RwUInt8 alpha;
};

struct RwRGBAReal
{
    RwReal red;
    RwReal green;
    RwReal blue;
    RwReal alpha;
};

struct RwSphere
{
    RwV3d  center;
    RwReal radius;
};

struct RwBBox
{
    RwV3d sup;
    RwV3d inf;
};

struct RwMatrix
{
    RwV3d    right;
    RwUInt32 flags;
    RwV3d    up;
    RwUInt32 pad1;
    RwV3d    at;
    RwUInt32 pad2;
    RwV3d    pos;
    RwUInt32 pad3;
};

// Opaque & Forward Structs
struct RwObject
{
    RwUInt8  type;
    RwUInt8  subType;
    RwUInt8  flags;
    RwUInt8  privateFlags;
    void    *parent;
};

struct RwLLLink
{
    struct RwLLLink *next;
    struct RwLLLink *prev;
};

struct RwLinkList
{
    struct RwLLLink link;
};

struct RwFrame
{
    RwObject          object;
    struct RwLLLink   link;
    RwMatrix          modelling;
    RwMatrix          ltm;
    struct RwLinkList childList;
    struct RwFrame   *child;
    struct RwFrame   *next;
    struct RwFrame   *root;
};

typedef RwFrame* (*RwFrameCallBack)(RwFrame *frame, void *data);

struct RwRaster;
typedef struct RwRaster RwRaster;

struct RwTexture;
typedef struct RwTexture RwTexture;

struct RwCamera;
typedef struct RwCamera RwCamera;

struct RpAtomic;
typedef struct RpAtomic RpAtomic;

struct RpWorld;
typedef struct RpWorld RpWorld;

struct RpClump;
typedef struct RpClump RpClump;

struct RwStream;
typedef struct RwStream RwStream;

// Video modu — FUN_0041af10 (CameraInit) 6 dword (24 byte) kopyalar; alan
// sırası binary'deki skorlamaya göre width/height/depth/flags + 2 bilinmeyen.
struct RwVideoMode
{
    RwInt32 width;
    RwInt32 height;
    RwInt32 depth;
    RwInt32 flags;
    RwInt32 unknown2;
    RwInt32 unknown3;
};
typedef struct RwVideoMode RwVideoMode;

// Alt sistem bilgisi (FUN_0050b1d0 — RwEngineGetSubSystemInfo)
#define rwSUBSYSTEMINFONAMELEN 80
struct RwSubSystemInfo
{
    RwChar name[rwSUBSYSTEMINFONAMELEN];
};
typedef struct RwSubSystemInfo RwSubSystemInfo;

// Raster type constants
enum RwRasterType
{
    rwRASTERTYPENORMAL     = 0x00,
    rwRASTERTYPEZBUFFER    = 0x01,
    rwRASTERTYPECAMERA     = 0x02,
    rwRASTERTYPETEXTURE    = 0x04,
    rwRASTERTYPECAMERASUB  = 0x05,
    rwRASTERTYPEMASK       = 0x07
};

// Function prototypes & helper macros
#define rwMatrixInitialize(m, t) RwMatrixSetIdentity(m)

void      RwMatrixSetIdentity(RwMatrix *matrix);
void      RwMatrixUpdate(RwMatrix *matrix);
void      RwMatrixMultiply(RwMatrix *matrixOut, const RwMatrix *matrixIn1, const RwMatrix *matrixIn2);
void      RwMatrixTranslate(RwMatrix *matrix, const RwV3d *translation, RwOpCombineType combineOp);
void      RwMatrixRotate(RwMatrix *matrix, const RwV3d *axis, RwReal angle, RwOpCombineType combineOp);
void      RwMatrixScale(RwMatrix *matrix, const RwV3d *scale, RwOpCombineType combineOp);

RwFrame*  RwFrameCreate(void);
RwBool    RwFrameDestroy(RwFrame *frame);
RwMatrix* RwFrameGetMatrix(RwFrame *frame);
RwMatrix* RwFrameGetLTM(RwFrame *frame);
RwFrame*  RwFrameTranslate(RwFrame *frame, const RwV3d *v, RwOpCombineType combineOp);
RwFrame*  RwFrameRotate(RwFrame *frame, const RwV3d *axis, RwReal angle, RwOpCombineType combineOp);
RwFrame*  RwFrameScale(RwFrame *frame, const RwV3d *v, RwOpCombineType combineOp);
RwFrame*  RwFrameUpdateObjects(RwFrame *frame);
RwFrame*  RwFrameAddChild(RwFrame *parent, RwFrame *child);
RwFrame*  RwFrameRemoveChild(RwFrame *child);
RwFrame*  RwFrameForAllChildren(RwFrame *frame, RwFrameCallBack callBack, void *data);
RwFrame*  RwFrameTransform(RwFrame *frame, const RwMatrix *matrix, RwOpCombineType combineOp);

RwRaster* RwRasterCreate(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags);
RwBool    RwRasterDestroy(RwRaster *raster);

RwCamera* RwCameraCreate(void);
RwBool    RwCameraDestroy(RwCamera *camera);
RwFrame*  RwCameraGetFrame(const RwCamera *camera);
RwCamera* RwCameraSetFrame(RwCamera *camera, RwFrame *frame);
RwCamera* RwCameraSetRaster(RwCamera *camera, RwRaster *raster);
RwCamera* RwCameraSetZRaster(RwCamera *camera, RwRaster *zRaster);
RwCamera* RwCameraSetNearClipPlane(RwCamera *camera, RwReal nearClip);
RwCamera* RwCameraSetFarClipPlane(RwCamera *camera, RwReal farClip);
RwCamera* RwCameraSetViewWindow(RwCamera *camera, const RwV2d *viewWindow);
RwCamera* RwCameraClear(RwCamera *camera, RwRGBA *colour, RwInt32 clearMode);
RwCamera* RwCameraBeginUpdate(RwCamera *camera);
RwCamera* RwCameraEndUpdate(RwCamera *camera);
RwCamera* RwCameraShowRaster(RwCamera *camera, void *pDev, RwUInt32 flags);

// Engine alt sistem / video modu sorguları (FUN_0050b060 komut dağıtıcı ailesi;
// CameraInit FUN_0041af10 bunlarla en iyi modu puanlar)
RwInt32   RwEngineGetNumSubSystems(void);
RwBool    RwEngineSetSubSystem(RwInt32 subSystemIndex);
RwBool    RwEngineGetSubSystemInfo(RwSubSystemInfo *info, RwInt32 subSystemIndex);
RwInt32   RwEngineGetNumVideoModes(void);
RwBool    RwEngineGetVideoModeInfo(RwVideoMode *videoMode, RwInt32 videoModeIndex);
RwBool    RwEngineSetVideoMode(RwInt32 videoModeIndex);

RwTexture* RwTextureRead(const RwChar *name, const RwChar *maskName);
RwBool     RwTextureDestroy(RwTexture *texture);
RwTexture* RwTextureSetRaster(RwTexture *texture, RwRaster *raster);
RwRaster*  RwTextureGetRaster(const RwTexture *texture);

// RpAtomic functions
RpAtomic*  RpAtomicCreate(void);
RwBool     RpAtomicDestroy(RpAtomic *atomic);
RwFrame*   RpAtomicGetFrame(const RpAtomic *atomic);
RpAtomic*  RpAtomicSetFrame(RpAtomic *atomic, RwFrame *frame);
RwUInt32   RpAtomicGetFlags(const RpAtomic *atomic);
RpAtomic*  RpAtomicSetFlags(RpAtomic *atomic, RwUInt32 flags);

RwStream* RwStreamOpen(RwInt32 type, RwInt32 accessType, const void *pData);
RwBool    RwStreamClose(RwStream *stream, void *pData);
RwUInt32  RwStreamRead(RwStream *stream, void *buffer, RwUInt32 length);
RwStream* RwStreamWrite(RwStream *stream, const void *buffer, RwUInt32 length);

// Chunk header info for stream processing
struct RwChunkHeaderInfo
{
    RwUInt32 type;
    RwUInt32 length;
    RwUInt32 version;
    RwUInt32 buildVersion;
    RwBool   isComplex;
};

RwBool    RwStreamReadChunkHeaderInfo(RwStream *stream, RwChunkHeaderInfo *chunkHeader);
RwUInt32  RwStreamSkip(RwStream *stream, RwUInt32 length);
RwUInt32  RwStreamTell(RwStream *stream); // debug/parse hizalama (Faz 4.6e)
RwBool    RwStreamSeekAbs(RwStream *stream, RwUInt32 position); // Faz 4.6e

#define rwSTREAMFILENAME 1
#define rwSTREAMREAD     1
// Oyunun kendi RW sürümü (3.6.0.3, libid 0x1803FFFF) txd chunk tipini
// 0x23 YAZAR (Faz 4.6e dosya kanıtı — embedded [0x23][len][libid] tüm
// .RWS txd kayıtlarında; klasik SDK 0x16 bu dosya ailesinde geçmez).
#define rwID_TEXDICTIONARY 0x0023
#define rwID_WORLD         0x000B
#define rwCAMERACLEARIMAGE 0x0001
#define rwCAMERACLEARZ     0x0002

// Camera functions
RwCamera* RwCameraGetCurrentCamera(void);
RwRaster* RwCameraGetRaster(RwCamera *camera);
RwRaster* RwCameraGetZRaster(RwCamera *camera);
RwReal    RwCameraGetNearClipPlane(const RwCamera *camera);
RwReal    RwCameraGetFarClipPlane(const RwCamera *camera);

// Raster functions
RwInt32   RwRasterGetWidth(const RwRaster *raster);
RwInt32   RwRasterGetHeight(const RwRaster *raster);
RwInt32   RwRasterGetDepth(const RwRaster *raster);
RwInt32   RwRasterGetStride(const RwRaster *raster);

// Im2D types and functions
struct RwIm2DVertex
{
    RwReal x, y, z;
    RwReal rhw;
    RwRGBA color;
    RwReal u, v;
};

typedef struct RwIm2DVertex RwIm2DVertex;

// Im2D vertex helper macros (inline in real RW SDK)
#define RwIm2DVertexSetScreenX(v, val)     ((v)->x = (val))
#define RwIm2DVertexSetScreenY(v, val)     ((v)->y = (val))
#define RwIm2DVertexSetScreenZ(v, val)     ((v)->z = (val))
#define RwIm2DVertexSetRecipCameraZ(v, val) ((v)->rhw = (val))
#define RwIm2DVertexSetIntRGBA(v, r, g, b, a) \
    do { \
        (v)->color.red = (r); \
        (v)->color.green = (g); \
        (v)->color.blue = (b); \
        (v)->color.alpha = (a); \
    } while(0)

#define rwPRIMTYPETRILIST    0x01
#define rwPRIMTYPETRISTRIP   0x04

// Render state types
enum RwRenderState
{
    rwRENDERSTATETEXTURERASTER    = 0,
    rwRENDERSTATEZTESTENABLE      = 1,
    rwRENDERSTATEZWRITEENABLE     = 2,
    rwRENDERSTATEVERTEXALPHAENABLE = 3,
    rwRENDERSTATESRCBLEND         = 4,
    rwRENDERSTATEDESTBLEND        = 5,
    rwRENDERSTATEFOGENABLE        = 6,
    rwRENDERSTATEFOGCOLOR         = 7,
    rwRENDERSTATEFOGTYPE          = 8,
    rwRENDERSTATEALPHATESTFUNC    = 9,
    rwRENDERSTATEALPHATESTREF     = 10,
    rwRENDERSTATECULLMODE         = 11,
    rwRENDERSTATESTENCILENABLE    = 12,
    rwRENDERSTATESTENCILFAIL      = 13,
    rwRENDERSTATESTENCILZFAIL     = 14,
    rwRENDERSTATESTENCILPASS      = 15,
    rwRENDERSTATESTENCILFUNC      = 16,
    rwRENDERSTATESTENCILREF       = 17,
    rwRENDERSTATESTENCILMASK      = 18,
    rwRENDERSTATESTENCILWRITEMASK = 19,
    rwRENDERSTATETEXTUREFILTER    = 20,
    rwRENDERSTATEFOGTABLE         = 21,
};

// Blend modes
enum RwBlendFunction
{
    rwBLENDZERO              = 1,
    rwBLENDONE               = 2,
    rwBLENDSRCCOLOR          = 3,
    rwBLENDINVSRCCOLOR       = 4,
    rwBLENDSRCALPHA          = 5,
    rwBLENDINVSRCALPHA       = 6,
    rwBLENDDESTALPHA         = 7,
    rwBLENDINVDESTALPHA      = 8,
    rwBLENDDESTCOLOR         = 9,
    rwBLENDINVDESTCOLOR      = 10,
    rwBLENDSRCALPHASAT       = 11,
};

RwBool    RwRenderStateSet(RwUInt32 state, void *value);
void*     RwRenderStateGet(RwUInt32 state);

RwBool    RwIm2DRenderPrimitive(RwUInt32 primType, const RwIm2DVertex *vertices, RwInt32 numVertices);
RwBool    RwIm2DRenderIndexedPrimitive(RwUInt32 primType, const RwUInt16 *indices, RwInt32 numIndices);

// Shade mode (rwRENDERSTATESHADEMODE değeri)
enum RwShadeMode
{
    rwSHADEMODEFLAT   = 0,
    rwSHADEMODEGOURAUD = 1,
};
#define rwRENDERSTATESHADEMODE 22

// Im3D types and functions (Immediate Mode 3D pipeline)
struct RwIm3DVertex
{
    RwV3d   position;
    RwReal  recipCameraZ;
    RwRGBA  color;
    RwReal  u, v;
};

typedef struct RwIm3DVertex RwIm3DVertex;

// Im3D vertex helper macros (inline in real RW SDK)
#define RwIm3DVertexSetPos(v, px, py, pz) \
    do { \
        (v)->position.x = (px); \
        (v)->position.y = (py); \
        (v)->position.z = (pz); \
    } while(0)
#define RwIm3DVertexSetRGBA(v, r, g, b, a) \
    do { \
        (v)->color.red = (r); \
        (v)->color.green = (g); \
        (v)->color.blue = (b); \
        (v)->color.alpha = (a); \
    } while(0)
#define RwIm3DVertexSetU(vtx, val) ((vtx)->u = (val))
#define RwIm3DVertexSetV(vtx, val) ((vtx)->v = (val))

// Im3D transform flags
#define rwIM3D_VERTEXUV    0x0001
#define rwIM3D_VERTEXRGBA  0x0002

RwBool RwIm3DTransform(RwIm3DVertex *vertices, RwUInt32 numVertices,
                       const RwMatrix *ltm, RwUInt32 flags);
void   RwIm3DEnd(void);
RwBool RwIm3DRenderPrimitive(RwUInt32 primType);
RwBool RwIm3DRenderIndexedPrimitive(RwUInt32 primType, RwUInt16 *indices, RwInt32 numIndices);

// Memory functions (RW global allocator — memoryhandler üzerinden geçer)
void*   RwMalloc(size_t size, RwUInt32 hint);
void    RwFree(void *mem);
void*   RwRealloc(void *mem, size_t size, RwUInt32 hint);
struct RwMemoryFunctions
{
    void* (*rwmalloc)(size_t size);
    void  (*rwfree)(void *ptr);
    void* (*rwrealloc)(void *ptr, size_t size);
    void* (*rwcalloc)(size_t numObj, size_t sizeObj);
};

// Engine functions
// FUN_0050b640 — binary: arg_0 = memFuncs, arg_4 = flags (byte, bit0 = ?),
//                arg_8 = resEntrySize (motor global'ine yazılıyor)
RwBool    RwEngineInit(RwMemoryFunctions *memFuncs, RwUInt32 flags, RwUInt32 resEntrySize);
RwBool    RwEngineTerm(void);
// FUN_00422960 — binary: argümansız (retn, arg_0 yok); startup.cpp RwEngineOpen() çağırır
RwBool    RwEngineOpen(void);
RwBool    RwEngineClose(void);
RwBool    RwEngineStart(void);
RwBool    RwEngineStop(void);
// Platform olay döngüsü (GL3: glfwPollEvents; NULL: no-op → FALSE).
RwBool    RwPlatformPollEvents(void);
// GL3: pencere kapatıldı mı (GLFWwindowShouldClose); NULL: hep FALSE.
RwBool    RwPlatformShouldClose(void);
// 4.6q freecam: tuş basılı mı (key = GLFW key kodu / ASCII 'W','A'...).
// GL3: glfwGetKey; NULL: hep FALSE.
RwBool    RwPlatformKeyDown(int key);
// GL3 ok tuş kodları (GLFW; ASCII tuşlar doğrudan bu değerlerle geçer).
#define RW_KEY_RIGHT 262
#define RW_KEY_LEFT  263
#define RW_KEY_DOWN  264
#define RW_KEY_UP    265
// 4.6r menü durum makinesi: onay/kaçış (GLFW).
#define RW_KEY_ESCAPE 256
#define RW_KEY_ENTER  257
// 4.6q mouse look: imleç yakalama (GLFW_CURSOR_DISABLED) + kare delta'sı.
// GL3: gerçek; NULL: FALSE.
RwBool    RwPlatformSetMouseCapture(RwBool capture);
RwBool    RwPlatformMouseDelta(int *pDX, int *pDY);
// FUN_0050f660 — RwEngineSetConfig: binary TEK pointer argüman alır
// (Ghidra: undefined4 FUN_0050f660(undefined4 *param_1); IDA: arg_0'ı
// 12 bayt okuyup motor nesnesine (dword_65D7A4 + dword_65D884 + 0xC)
// kopyalar, her zaman 1 döner). sub_41A680 çağırmadan önce yığına
// {1e-5, 1e-5, 0.01f} yazar (startup.cpp "Adım 12" ile aynı değerler).
struct RwEngineConfig
{
    RwReal nearClip;
    RwReal farClip;
    RwReal fog;
};
RwBool    RwEngineSetConfig(const RwEngineConfig *pConfig);
// sub_41CDF0 — bağlam sorgusu (binary: dword_6361B4 = sub_41CE00 render
// hedefi; NULL ise ResizeCameraRasters FALSE döner). Geçici karşılık:
// durum makinesi START == TRUE (gerçek ctx Faz 4.4b'de restor edilecek).
RwBool    RwEngineIsRunning(void);

struct RwFreeList
{
    RwInt32  itemsPerBlock;
    RwInt32  numBlocks;
    RwInt32  maxItems;
    RwInt32  blockSize;
    void    *blocks;
    void    *freeList;
    void    *freeListEnd;
    RwUInt32 flags;
};

struct RwGlobals
{
    void    *currentCamera;
    void    *memoryFunctions;
};

RwGlobals* RwGetGlobals(void);

struct RwTexDictionary;
typedef struct RwTexDictionary RwTexDictionary;

RwTexDictionary* RwTexDictionaryCreate(void);
RwBool           RwTexDictionaryDestroy(RwTexDictionary *dict);
RwTexDictionary* RwTexDictionarySetCurrent(RwTexDictionary *dict);
RwTexDictionary* RwTexDictionaryGetCurrent(void);
RwTexDictionary* RwTexDictionaryStreamRead(RwStream *stream);
RwBool           RwStreamFindChunk(RwStream *stream, RwUInt32 type, RwUInt32 *lengthOut, RwUInt32 *versionOut);

typedef void* (*RwPluginObjectConstructor)(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);
typedef void* (*RwPluginObjectDestructor)(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);
typedef void* (*RwPluginObjectCopy)(void *dstObject, const void *srcObject, RwInt32 offsetInObject, RwInt32 sizeInObject);

RwInt32   RwEngineRegisterPlugin(RwInt32 size, RwUInt32 pluginID,
                                 RwPluginObjectConstructor constructCB,
                                 RwPluginObjectDestructor destructCB,
                                 RwPluginObjectCopy copyCB);

// Texture functions
RwTexture* RwTextureCreate(RwRaster *raster);
RwBool     RwTextureDestroy(RwTexture *texture);
RwTexture* RwTextureSetRaster(RwTexture *texture, RwRaster *raster);
RwRaster*  RwTextureGetRaster(const RwTexture *texture);

// String functions
const RwChar* RwStrcpy(RwChar *dest, const RwChar *src);
RwInt32       RwStrlen(const RwChar *str);

// Vector math
void RwV3dNormalize(RwV3d *out, const RwV3d *in);
RwReal RwV3dDotProduct(const RwV3d *a, const RwV3d *b);
void RwV3dCrossProduct(RwV3d *out, const RwV3d *a, const RwV3d *b);

// Common chunk IDs
#define rwID_CLUMP          0x0010
#define rwID_ATOMIC         0x000E
#define rwID_LIGHT          0x0012
#define rwID_CAMERA         0x0005
#define rwID_TEXTURE        0x0006
#define rwID_TEXDICTIONARY  0x0023
#define rwID_WORLD          0x000B
#define rwID_WORLDSECTOR    0x0014
#define rwID_PLANESECTOR    0x0015
#define rwID_SPLINE         0x001C
#define rwID_MATRIX         0x001E
#define rwID_FRAMELIST      0x001F
#define rwID_GEOMETRY       0x0008
#define rwID_MATERIALEFFECTS 0x0024
#define rwID_ADC            0x0025
#define rwID_UVMANIMATION   0x0026
#define rwID_SKYMIPMAP      0x0027
#define rwID_HANIMANIMATION 0x011E
#define rwID_USERDATA       0x002A

// Error handling
typedef enum RwErrorCode
{
    rwERROR_NONE = 0,
    rwERROR_NOTINITIALIZED,
    rwERROR_INVALIDPARAM,
    rwERROR_ALLOCMEM,
    rwERROR_OUTOFMEMORY,
    rwERROR_DRIVERNOTFOUND,
    rwERROR_IMMUTABLE,
    rwERROR_BADOBJECT,
    rwERROR_NOTFOUND,
    rwERROR_NOTENABLED,
    rwERROR_INVALIDOBJECT,
    rwERROR_WRONGDEVICE,
    rwERROR_BADDATA,
    rwERROR_NOSUCHFILE,
    rwERROR_FILENOTFOUND,
    rwERROR_FILEOPEN,
    rwERROR_FILEREAD,
    rwERROR_FILEWRITE,
    rwERROR_FILESEEK,
    rwERROR_FILETELL,
    rwERROR_BADSTREAM,
    rwERROR_BADCHUNK,
    rwERROR_CHUNKNOTFOUND,
    rwERROR_BADVERSION,
} RwErrorCode;

RwErrorCode RwGetErrorCode(void);
const RwChar* RwGetErrorString(RwErrorCode code);

#ifdef __cplusplus
}
#endif

#endif // RWCORE_H

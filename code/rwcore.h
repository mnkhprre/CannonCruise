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

struct RwStream;
typedef struct RwStream RwStream;

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

RwRaster* RwRasterCreate(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags);
RwBool    RwRasterDestroy(RwRaster *raster);

RwCamera* RwCameraCreate(void);
RwBool    RwCameraDestroy(RwCamera *camera);
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

RwTexture* RwTextureRead(const RwChar *name, const RwChar *maskName);
RwBool     RwTextureDestroy(RwTexture *texture);
RwTexture* RwTextureSetRaster(RwTexture *texture, RwRaster *raster);

RwStream* RwStreamOpen(RwInt32 type, RwInt32 accessType, const void *pData);
RwBool    RwStreamClose(RwStream *stream, void *pData);
RwUInt32  RwStreamRead(RwStream *stream, void *buffer, RwUInt32 length);
RwStream* RwStreamWrite(RwStream *stream, const void *buffer, RwUInt32 length);

typedef void* (*RwPluginObjectConstructor)(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);
typedef void* (*RwPluginObjectDestructor)(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);
typedef void* (*RwPluginObjectCopy)(void *dstObject, const void *srcObject, RwInt32 offsetInObject, RwInt32 sizeInObject);

RwInt32   RwEngineRegisterPlugin(RwInt32 size, RwUInt32 pluginID,
                                 RwPluginObjectConstructor constructCB,
                                 RwPluginObjectDestructor destructCB,
                                 RwPluginObjectCopy copyCB);

#ifdef __cplusplus
}
#endif

#endif // RWCORE_H

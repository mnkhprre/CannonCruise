#ifndef RPWORLD_H
#define RPWORLD_H

// ============================================================================
// RenderWare World Plugin Stub (rpworld.h)
// ============================================================================

#include "rwcore.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RpClump;
typedef struct RpClump RpClump;

struct RpAtomic;
typedef struct RpAtomic RpAtomic;

struct RpGeometry;
typedef struct RpGeometry RpGeometry;

struct RpMaterial;
typedef struct RpMaterial RpMaterial;

struct RpWorld;
typedef struct RpWorld RpWorld;

struct RpLight;
typedef struct RpLight RpLight;

struct RpSkin;
typedef struct RpSkin RpSkin;

struct RpHAnimHierarchy;
typedef struct RpHAnimHierarchy RpHAnimHierarchy;

typedef RpClump* (*RpClumpCallBack)(RpClump *clump, void *data);
typedef RpAtomic* (*RpAtomicCallBack)(RpAtomic *atomic, void *data);

// Clump API
RpClump*    RpClumpCreate(void);
RwBool      RpClumpDestroy(RpClump *clump);
RpClump*    RpClumpClone(RpClump *clump);
void        RpClumpRender(RpClump *clump);
RpClump*    RpClumpForAllAtomics(RpClump *clump, RpAtomicCallBack callback, void *data);
RwFrame*    RpClumpGetFrame(RpClump *clump);
RpClump*    RpClumpSetFrame(RpClump *clump, RwFrame *frame);
RpClump*    RpClumpStreamRead(RwStream *stream);
// CollisionTrigger (phantom AABB) için eklendi — librw'de gerçek veri döner
const RwSphere* RpClumpGetBoundingSphere(const RpClump *clump);

// Atomic API
RpAtomic*   RpAtomicCreate(void);
RwBool      RpAtomicDestroy(RpAtomic *atomic);
RpGeometry* RpAtomicGetGeometry(const RpAtomic *atomic);
RpAtomic*   RpAtomicSetGeometry(RpAtomic *atomic, RpGeometry *geometry, RwUInt32 flags);
RwFrame*    RpAtomicGetFrame(const RpAtomic *atomic);
RpAtomic*   RpAtomicSetFrame(RpAtomic *atomic, RwFrame *frame);
RpAtomic*   RpAtomicSetFlags(RpAtomic *atomic, RwUInt32 flags);
RwUInt32    RpAtomicGetFlags(const RpAtomic *atomic);

// Skin plugin atomic helpers
RpSkin*           RpSkinAtomicGetSkin(const RpAtomic *atomic);
RpHAnimHierarchy* RpSkinAtomicGetHAnimHierarchy(const RpAtomic *atomic);
RpAtomic*         RpSkinAtomicSetHAnimHierarchy(RpAtomic *atomic, RpHAnimHierarchy *hierarchy);

// Geometry API
RpGeometry*     RpGeometryCreate(RwInt32 numVerts, RwInt32 numTriangles, RwUInt32 format);
RwBool          RpGeometryDestroy(RpGeometry *geometry);
const RwSphere* RpGeometryGetBoundingSphere(const RpGeometry *geometry);
// vertex/tükenme erişimi — CollisionGeometry (Clump→şekil) için eklendi;
// librw entegrasyonunda gerçek veri döner (stub: 0/0/NULL)
RwInt32         RpGeometryGetNumVertices(const RpGeometry *geometry);
RwInt32         RpGeometryGetNumTriangles(const RpGeometry *geometry);
RwV3d*          RpGeometryGetVertices(RpGeometry *geometry);
RwUInt16*       RpGeometryGetTriangleIndices(RpGeometry *geometry);
RwUInt32        RpGeometryGetFlags(const RpGeometry *geometry);

// World API
RpWorld*    RpWorldCreate(RwBBox *boundingBox);
RwBool      RpWorldDestroy(RpWorld *world);
RpWorld*    RpWorldStreamRead(RwStream *stream);
void        RpWorldRender(RpWorld *world);
RpWorld*    RpWorldAddClump(RpWorld *world, RpClump *clump);
RpWorld*    RpWorldRemoveClump(RpWorld *world, RpClump *clump);
RpWorld*    RpWorldAddCamera(RpWorld *world, RwCamera *camera);
RpWorld*    RpWorldRemoveCamera(RpWorld *world, RwCamera *camera);
RpWorld*    RpWorldAddLight(RpWorld *world, RpLight *light);
RpWorld*    RpWorldRemoveLight(RpWorld *world, RpLight *light);

// Material & Plugin API
RwInt32     RpMaterialRegisterPlugin(RwInt32 size, RwUInt32 pluginID,
                                     RwPluginObjectConstructor constructCB,
                                     RwPluginObjectDestructor destructCB,
                                     RwPluginObjectCopy copyCB);
RwBool      RpWorldPluginAttach(void);

#ifdef __cplusplus
}
#endif

#endif // RPWORLD_H

#ifndef RPHANIM_H
#define RPHANIM_H

#include "rwcore.h"
#include "rpworld.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RpHAnimHierarchy
{
    RwInt32 numNodes;
    void*   pNodeInfo;
    void*   pMatrixArray;
};
typedef struct RpHAnimHierarchy RpHAnimHierarchy;

struct RtAnimAnimation;
typedef struct RtAnimAnimation RtAnimAnimation;
typedef struct RtAnimAnimation RpHAnimAnimation;

RwBool             RpHAnimPluginAttach(void);
void               RpHAnimHierarchyUpdateMatrices(RpHAnimHierarchy *hierarchy);
void               RpHAnimHierarchyAddAnimTime(RpHAnimHierarchy *hierarchy, RwReal time);
void               RpHAnimHierarchySetCurrentAnim(RpHAnimHierarchy *hierarchy, RtAnimAnimation *anim);
void               RpHAnimHierarchyBlendHierarchy(RpHAnimHierarchy *dest, RpHAnimHierarchy *src, RwReal alpha);
RpHAnimHierarchy*  RpSkinSafeAtomicGetHAnimHierarchy(RpClump *clump);
void               RpSkinSafeClumpSetHAnimHierarchy(RpClump *clump, RpHAnimHierarchy *hierarchy);

#ifdef __cplusplus
}
#endif

#endif // RPHANIM_H

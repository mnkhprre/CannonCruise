// ============================================================================
// CannonCruise - Faz 4.6p: WorldSpawn implementation
// ============================================================================

#include "WorldSpawn.h"

#include "AudioStreamEntity.h"
#include "BehaviourLoader.h"
#include "CameraState.h"
#include "PhysicsWorld.h"
#include "RenderBehaviourWorld.h"
#include "RwStudioStream.h"

#include <cstdio>
#include <cstring>

namespace {

// ---- fabrika + apply adaptörleri (binary: factory + vtable slot1) ----
// Test deseninin birebir aynısı (behaviourloader_test.cpp); apply
// singleton hedefine akar (binary: apply DAT_00635648).

void *CreateWbRender() {
    CRenderBehaviourSub *pSub = new CRenderBehaviourSub;
    std::memset(pSub, 0, sizeof(*pSub));
    return pSub;
}

RwBool ApplyWbRender(void *, RwUInt32 id, const RwUInt8 *pValue) {
    RwUInt32 bits = 0;
    std::memcpy(&bits, pValue, 4);
    return CRenderBehaviourWorld::Instance()->ApplyWbRenderAttribute(id, bits);
}

void *CreateWbCamera() {
    CCameraState *pCam = new CCameraState;
    std::memset(pCam, 0, sizeof(*pCam));
    return pCam;
}

RwBool ApplyWbCamera(void *pObject, RwUInt32 id, const RwUInt8 *pValue) {
    CCameraState *pCam = static_cast<CCameraState *>(pObject);
    if (CCameraStateApplyVec3(pCam, id, pValue) == TRUE) {
        return TRUE; // VEC3 kaydı (12B kopya)
    }
    RwUInt32 bits = 0;
    std::memcpy(&bits, pValue, 4);
    return pCam->ApplyAttribute(id, bits);
}

void *CreateAudio() {
    CAudioStreamEntity *pAudio = new CAudioStreamEntity;
    CAudioStreamEntityInit(pAudio);
    return pAudio;
}

RwBool ApplyAudio(void *pObject, RwUInt32 id, const RwUInt8 *pValue) {
    return CAudioStreamEntityApplyAttribute(
        static_cast<CAudioStreamEntity *>(pObject), id, pValue);
}

void *CreatePhysicsWorld() { return new CPhysicsWorldBehaviour; }

// CPhysicsWorldBehaviour apply: binary'de property yok — tetikleme akışına
// mesaj-driven (HandleEvents); port no-op döner.
RwBool ApplyPhysicsWorld(void *, RwUInt32, const RwUInt8 *) { return TRUE; }

} // namespace

RwBool GameBehaviours_Register() {
    BehaviourLoader::UnregisterAll();

    BehaviourClassEntry entries[] = {
        {"CWbRenderBehavior", sizeof(CRenderBehaviourSub), &CreateWbRender,
         &ApplyWbRender},
        {"CWbCameraBehavior", sizeof(CCameraState), &CreateWbCamera,
         &ApplyWbCamera},
        {"CAudioStreamEntity", sizeof(CAudioStreamEntity), &CreateAudio,
         &ApplyAudio},
        {"CPhysicsWorldBehaviour", sizeof(CPhysicsWorldBehaviour),
         &CreatePhysicsWorld, &ApplyPhysicsWorld},
    };
    for (RwUInt32 i = 0; i < sizeof(entries) / sizeof(entries[0]); ++i) {
        if (BehaviourLoader::Register(&entries[i]) != TRUE) {
            return FALSE;
        }
    }
    return TRUE;
}

RwInt32 WorldSpawn_FromFile(const RwChar *pPath, WorldSpawnStats *pStats) {
    WorldSpawnStats stats;
    std::memset(&stats, 0, sizeof(stats));
    if (pStats) {
        std::memset(pStats, 0, sizeof(*pStats));
    }

    RwStudioFile *pFile = RwStudioFileOpen(pPath);
    if (!pFile) {
        return -1;
    }
    RwStudioAsset asset;
    RwInt32 totalApplied = 0;
    while (RwStudioFileNextAsset(pFile, &asset) == TRUE) {
        const RwStudioObject *pObj = RwStudioFileGetObject(pFile);
        if (!pObj) {
            continue;
        }
        ++stats.objectRecords;
        void *pInstances[64];
        const RwInt32 applied = BehaviourLoader::ProcessObject(
            pObj, pInstances,
            (RwInt32)(sizeof(pInstances) / sizeof(pInstances[0])));
        if (applied > 0) {
            stats.instancesCreated += (RwUInt32)applied;
            totalApplied += applied;
        }
        // bilinmeyen sınıflar: factory miss (binary: property atla)
        stats.instancesSkipped +=
            pObj->behaviourCount - (RwUInt32)(applied > 0 ? applied : 0);
    }
    RwStudioFileClose(pFile);

    if (pStats) {
        *pStats = stats;
    }
    return totalApplied;
}

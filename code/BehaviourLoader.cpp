// ============================================================================
// CannonCruise - BehaviourLoader implementation
// Binary: FUN_00411050 (Process) + CClassFactory::Create + apply virtuals
// ============================================================================

#include "BehaviourLoader.h"

#include "attributehandler/cclassfactory.h"

#include <cstring>

namespace {
const RwInt32 kMaxClasses = 16;
BehaviourClassEntry s_Entries[kMaxClasses];
RwInt32 s_EntryCount = 0;
} // namespace

const BehaviourClassEntry *BehaviourLoader::sm_pEntries = s_Entries;
RwInt32 BehaviourLoader::sm_entryCount = 0;

RwBool BehaviourLoader::Register(const BehaviourClassEntry *pEntry) {
    if (!pEntry || !pEntry->className || !pEntry->create || !pEntry->apply) {
        return FALSE;
    }
    if (s_EntryCount >= kMaxClasses) {
        return FALSE;
    }
    s_Entries[s_EntryCount] = *pEntry;
    ++s_EntryCount;
    sm_entryCount = s_EntryCount;

    // In binary, create is registered with CClassFactory (FUN_00413700 Register)
    RWS::CClassFactory::Register(
        pEntry->className,
        (RWS::ClassCreationFunc)pEntry->create, pEntry->size, 1);
    return TRUE;
}

void BehaviourLoader::UnregisterAll() {
    for (RwInt32 i = 0; i < s_EntryCount; ++i) {
        RWS::CClassFactory::UnRegister(s_Entries[i].className);
    }
    s_EntryCount = 0;
    sm_entryCount = 0;
}

void *BehaviourLoader::Create(const char *pClassName) {
    if (!pClassName) {
        return 0;
    }
    return RWS::CClassFactory::Create(pClassName);
}

RwInt32 BehaviourLoader::ProcessObject(const RwStudioObject *pObject,
                                       void **ppInstances,
                                       RwInt32 maxInstances) {
    if (!pObject) {
        return -1;
    }
    RwInt32 applied = 0;
    for (RwUInt32 b = 0; b < pObject->behaviourCount; ++b) {
        const RwStudioBehaviour *pBeh = &pObject->behaviours[b];
        // class name -> factory (binary: CClassFactory::Create)
        void *pInstance = Create(pBeh->className);
        if (!pInstance) {
            continue; // unknown class: Process property continues to skip
        }
        // apply virtual (vtable slot1): property registrations
        for (RwUInt32 i = 0; i < s_EntryCount; ++i) {
            if (std::strcmp(s_Entries[i].className, pBeh->className) != 0) {
                continue;
            }
            for (RwUInt32 p = 0; p < pBeh->propCount; ++p) {
                const RwStudioProperty &prop = pBeh->props[p];
                s_Entries[i].apply(pInstance, prop.id, prop.value);
            }
            break;
        }
        if (ppInstances && applied < maxInstances) {
            ppInstances[applied] = pInstance;
        }
        ++applied;
    }
    return applied;
}

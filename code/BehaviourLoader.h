// ============================================================================
// CannonCruise - BehaviourLoader (0x0704 create-by-name dispatch)
//
// Binary: FUN_00411010 (CEventHandler ctor(stream, className) -> immediately
// Process(false)), FUN_00411050 (Process: 0x20/0x40 skip; 0x80000000 ->
// *(DAT_0065d7a4+0x114)(this+4, rec+8) class name comparison (strcmp
// semantics, 0=match); stops at first property of matching behaviour; non-matching
// properties skipped), FUN_004110e0 (Advance + Process(1) —
// transition to next behaviour), CClassFactory::Create (FUN_004134xx family,
// FUN_00413700 = Register) and per-class apply virtual (vtable slot1:
// FUN_004c3e40 / sub_4C3450 / sub_458BD0).
//
// This port models dispatch glue and connects existing apply ports end-to-end:
// class name -> create factory -> apply all behaviour's properties.
// Unknown class skipped (binary: factory NULL -> Process property
// continues to skip).
// ============================================================================

#ifndef CANNONCRUISE_BEHAVIOURLOADER_H
#define CANNONCRUISE_BEHAVIOURLOADER_H

#include "RwStudioStream.h"
#include "rwcore.h"

/**
 * @struct BehaviourClassEntry
 * @brief Factory + apply record of a registered behaviour class.
 *
 * In binary, this pair is distributed as CClassFactory::Register (create) + vtable slot1
 * (apply); port collects them in single table.
 */
struct BehaviourClassEntry {
    const char *className; ///< "CWbRenderBehavior" etc.
    RwUInt32 size;         ///< object size (binary Register size)
    void *(*create)();     ///< factory: zeroed+Init object
    /// apply virtual (slot1): apply property record
    RwBool (*apply)(void *pObject, RwUInt32 id, const RwUInt8 *pValue);
};

/**
 * @class BehaviourLoader
 * @brief 0x0704 OBJECTRECORD → class name → factory → apply dispatch'i.
 *
 * Binary flow (CEventHandler::Process + class applies):
 * - each behaviour: class name (0x20/0x80000000 segment) -> CClassFactory
 *   lookup -> create; property records flow to apply virtual.
 * - this port: runs same flow over RwStudioObject (parsed).
 */
class BehaviourLoader {
public:
    /**
     * @brief Registers class: CClassFactory::Register + apply table.
     * @return TRUE if registration successful.
     */
    static RwBool Register(const BehaviourClassEntry *pEntry);

    /// Clears all registrations (test teardown)
    static void UnregisterAll();

    /**
     * @brief Creates object by class name (CClassFactory bridge).
     * @return Object or NULL (unknown class).
     */
    static void *Create(const char *pClassName);

    /**
     * @brief Creates+applies all behaviours in an OBJECTRECORD.
     *
     * For each behaviour: Create(className); if NULL skip (binary:
     * factory missing -> properties skipped). Otherwise records apply call
     * in ppInstances[].
     *
     * @param pObject      Parsed object (RwStudioFileGetObject()).
     * @param ppInstances  Created object pointers (output).
     * @param maxInstances ppInstances capacity.
     * @return Number of behaviours applied (-1: pObject NULL).
     */
    static RwInt32 ProcessObject(const RwStudioObject *pObject,
                                 void **ppInstances, RwInt32 maxInstances);

private:
    static const BehaviourClassEntry *sm_pEntries;
    static RwInt32 sm_entryCount;
};

#endif // CANNONCRUISE_BEHAVIOURLOADER_H

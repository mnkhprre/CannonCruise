// ============================================================================
// CannonCruise - BehaviourLoader (0x0704 create-by-name dispatch)
//
// Binary: FUN_00411010 (CEventHandler ctor(stream, className) → hemen
// Process(false)), FUN_00411050 (Process: 0x20/0x40 skip; 0x80000000 →
// *(DAT_0065d7a4+0x114)(this+4, rec+8) sınıf adı karşılaştırma (strcmp
// semantiği, 0=eşleşme); eşleşen behaviour'ın ilk property'sinde DUR; eşleş-
// meyen property'ler atlanır), FUN_004110e0 (Advance + Process(1) —
// sonraki behaviour'a geçiş), CClassFactory::Create (FUN_004134xx ailesi,
// FUN_00413700 = Register) ve sınıf başına apply virtual'ı (vtable slot1:
// FUN_004c3e40 / sub_4C3450 / sub_458BD0).
//
// Bu port dispatch glue'sunu modelleyip mevcut apply portlarını uçtan uca
// bağlar: sınıf adı → fabrika oluştur → behaviour'ın tüm property'lerini
// apply. Bilinmeyen sınıf atlanır (binary: factory NULL → Process property
// atlamaya devam eder).
// ============================================================================

#ifndef CANNONCRUISE_BEHAVIOURLOADER_H
#define CANNONCRUISE_BEHAVIOURLOADER_H

#include "RwStudioStream.h"
#include "rwcore.h"

/**
 * @struct BehaviourClassEntry
 * @brief Kayıtlı bir behaviour sınıfının fabrika + apply kaydı.
 *
 * Binary'de bu ikili CClassFactory::Register (create) + vtable slot1
 * (apply) olarak dağılıktır; port tek tabloda toplar.
 */
struct BehaviourClassEntry {
    const char *className; ///< "CWbRenderBehavior" vb.
    RwUInt32 size;         ///< nesne boyutu (binary Register boyutu)
    void *(*create)();     ///< fabrika: sıfırlanmış+Init nesne
    /// apply virtual (slot1): property kaydını uygula
    RwBool (*apply)(void *pObject, RwUInt32 id, const RwUInt8 *pValue);
};

/**
 * @class BehaviourLoader
 * @brief 0x0704 OBJECTRECORD → class name → factory → apply dispatch'i.
 *
 * Binary akışı (CEventHandler::Process + sınıf apply'ları):
 * - her behaviour: sınıf adı (0x20/0x80000000 segmenti) → CClassFactory
 *   lookup → create; property kayıtları apply virtual'ına akar.
 * - bu port: RwStudioObject (parse edilmiş) üzerinden aynı akışı yürütür.
 */
class BehaviourLoader {
public:
    /**
     * @brief Sınıfı kaydeder: CClassFactory::Register + apply tablosu.
     * @return TRUE ise kayıt başarılı.
     */
    static RwBool Register(const BehaviourClassEntry *pEntry);

    /// Tüm kayıtları temizler (test teardown)
    static void UnregisterAll();

    /**
     * @brief Sınıf adına göre nesne oluşturur (CClassFactory köprüsü).
     * @return Nesne veya NULL (bilinmeyen sınıf).
     */
    static void *Create(const char *pClassName);

    /**
     * @brief Bir OBJECTRECORD'daki tüm behaviour'ları create+apply eder.
     *
     * Her behaviour için: Create(className); NULL ise atla (binary:
     * factory yok → property'ler atlanır). Değilse apply çağrısını
     * ppInstances[]'e kaydeder.
     *
     * @param pObject      Parse edilmiş nesne (RwStudioFileGetObject()).
     * @param ppInstances  Oluşturulan nesne işaretçileri (çıktı).
     * @param maxInstances ppInstances kapasitesi.
     * @return Uygulanan behaviour sayısı (-1: pObject NULL).
     */
    static RwInt32 ProcessObject(const RwStudioObject *pObject,
                                 void **ppInstances, RwInt32 maxInstances);

private:
    static const BehaviourClassEntry *sm_pEntries;
    static RwInt32 sm_entryCount;
};

#endif // CANNONCRUISE_BEHAVIOURLOADER_H

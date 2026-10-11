// ============================================================================
// CannonCruise - Faz 4.6p: 0x0704 nesne kayıtlarından dünya spawn zinciri
// Binary: CEventHandler::Process döngüsü (FUN_00411050) — sınıf adı →
// CClassFactory::Create → apply virtual (vtable slot1).
//
// Bu modül iki iş yapar:
//   1. GameBehaviours_Register(): bilinen behaviour sınıflarını
//      BehaviourLoader'a kaydeder (render/camera/audio/physics portları).
//   2. WorldSpawn_FromFile(): RWS dosyasındaki tüm OBJECTRECORD
//      kayıtlarını walk edip ProcessObject ile örnek üretir; bilinmeyen
//      sınıflar binary'de olduğu gibi sessiz atlanır.
//
// Sahiplik: oluşturulan örnekler süreç sonuna dek yaşar (binary'de sahne
// teardown'ı CClassFactory delete çağırır — port 4.6s'e kadar leak bilinçli).
// ============================================================================

#ifndef CANNONCRUISE_WORLDSPAWN_H
#define CANNONCRUISE_WORLDSPAWN_H

#include "rwcore.h"

/// WORLDSPAWN istatistikleri (RWS walk çıktısı)
struct WorldSpawnStats {
    RwUInt32 objectRecords;    ///< walk edilen 0x0704 kayıtları
    RwUInt32 instancesCreated; ///< Create() başarılı (bilinen sınıflar)
    RwUInt32 instancesSkipped; ///< factory yok → binary: property atla
};

/**
 * @brief Bilinen behaviour sınıflarını kaydeder (binary Register deseni).
 *
 * CWbRenderBehavior / CWbCameraBehavior / CAudioStreamEntity /
 * CPhysicsWorldBehaviour — apply adaptörleri 4.6h-j portlarına bağlı.
 * @return TRUE kayıt başarılı (tekrar çağrılabilir — önce UnregisterAll).
 */
RwBool GameBehaviours_Register();

/**
 * @brief RWS dosyasındaki tüm 0x0704 kayıtlarını create+apply eder.
 * @param pPath  RWS dosya yolu.
 * @param pStats İstatistik (opsiyonel).
 * @return Uygulanan behaviour sayısı (-1: dosya açılamadı).
 */
RwInt32 WorldSpawn_FromFile(const RwChar *pPath, WorldSpawnStats *pStats);

#endif // CANNONCRUISE_WORLDSPAWN_H

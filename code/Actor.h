#pragma once

// ============================================================================
// CannonCruise - Base Actor (Actor.h)
// Original path: D:\Projects\CannonCruisePC\code\Actor.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00401000 - FUN_00401200 (CActor vtable bölgesi)
// ============================================================================

#include "AnimController.h"
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <rpworld.h>
#include <rwcore.h>

namespace RWS {

/**
 * @class CActor
 * @brief RenderWare Studio tabanlı oyun nesneleri için temel aktör sınıfı.
 *
 * CActor, `CAttributeHandler` ve `CEventHandler`'dan türeyen ve
 * RpClump (3D mesh) ile CAnimController (animasyon) yönetimini üstlenen
 * temel sınıftır. Soldier, Bird, Fish, FernandoActor gibi tüm sahne varlıkları
 * bu sınıftan türer.
 *
 * **Bellek düzeni (binary offset):**
 * - `+0x00` vtable pointer
 * - `+0x04` CAttributeHandler base
 * - `+0x14` CEventHandler base
 * - `+0x24` m_pAnimController
 * - `+0x28` m_pClump
 * - `+0x2C` m_bAddedToWorld
 *
 * **Binary referansları:**
 * - Ctor: `FUN_00401000`
 * - Dtor: `FUN_00401080`
 * - HandleAttributes: `FUN_00401100`
 * - AddToWorld/RemoveFromWorld: `FUN_00401140`/`FUN_00401180`
 */
class CActor : public CAttributeHandler, public CEventHandler {
public:
  RWS_MAKENEWCLASS(CActor);
  RWS_DECLARE_CLASS(CActor);

  /**
   * @brief Yapıcı — RWS attribute paketi ile nesneyi başlatır.
   * @param packet İlk clump ve transform verilerini taşıyan attribute paketi.
   *
   * CAnimController oluşturulur; paketten clump verileri çıkarılırsa
   * animasyon hedefi buna atanır. `iMsgRunningTick` mesajına kaydolur.
   */
  CActor(const CAttributePacket &packet);

  /**
   * @brief Yıkıcı — animasyon denetleyicisini ve clump'ı serbest bırakır.
   *
   * `iMsgRunningTick` kaydını iptal eder, `RemoveFromWorld()` çağırır,
   * ardından CAnimController ve RpClump belleği serbest bırakır.
   */
  virtual ~CActor();

  /**
   * @brief RWS attribute paketini işler; clump ve animasyon güncellenir.
   * @param packet Güncel transform/animasyon verilerini taşıyan paket.
   */
  virtual void HandleAttributes(const CAttributePacket &packet);

  /**
   * @brief Gelen olay mesajını işler (clump değiştirme gibi).
   * @param msg İşlenecek CMsg olayı.
   */
  virtual void HandleEvents(CMsg &msg);

  /**
   * @brief Clump'ı RpWorld sahnesine ekler (tekrar eklememe koruması ile).
   */
  void AddToWorld();

  /**
   * @brief Clump'ı RpWorld sahnesinden çıkarır.
   */
  void RemoveFromWorld();

  /**
   * @brief Bu aktörün 3D mesh'ini döndürür.
   * @return RpClump* — geçerli mesh işaretçisi veya nullptr.
   */
  RpClump *GetClump() const { return m_pClump; }

  /**
   * @brief Animasyon denetleyicisini döndürür.
   * @return CAnimController* — animasyon denetleyici işaretçisi.
   */
  CAnimController *GetAnimController() const { return m_pAnimController; }

  /**
   * @brief Aktörün sahneye eklenip eklenmediğini döndürür.
   * @return RwBool — TRUE ise RpWorld içinde.
   */
  RwBool IsAddedToWorld() const { return m_bAddedToWorld; }

protected:
  CAnimController *m_pAnimController; ///< Animasyon denetleyici (+0x24)
  RpClump *m_pClump;                  ///< 3D model mesh (+0x28)
  RwBool m_bAddedToWorld;             ///< Dünyaya eklenme bayrağı (+0x2C)
};

} // namespace RWS

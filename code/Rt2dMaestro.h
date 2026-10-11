#ifndef RT2DMAESTRO_H
#define RT2DMAESTRO_H

// ============================================================================
// CannonCruise - Rt2dMaestro gerçek implementasyonu (Rt2dMaestro.h)
//
// Menu.anm stream'inden oluşturulan menü sahne yöneticisi: parsed node
// tablosu + seçim durumu + Im2D placeholder render (çerçeve + seçim çubuğu).
// CMenuBehavior'ın m_pMaestro'sunu besler (FUN_00443800/FUN_00443c20).
// ============================================================================

#include "Rt2dMaestroStream.h"
#include <rt2danim.h>

struct Rt2dMaestro
{
    Rt2dMaestroStreamInfo info;   ///< parse edilmiş stream
    RwReal  time;                 ///< saniye — Update biriktirir
    RwInt32 selection;            ///< info.nodes[] indeksi (aktif komut)
    RwInt32 commandCount;         ///< type=6 komut sayısı (menü öğeleri)
    RwInt32 commands[rwrt2dMAX_NODES]; ///< komut node indeksleri
};

#endif // RT2DMAESTRO_H

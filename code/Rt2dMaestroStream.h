#ifndef RT2DMAESTROSTREAM_H
#define RT2DMAESTROSTREAM_H

// ============================================================================
// CannonCruise - Rt2dMaestro .anm stream parser (Rt2dMaestroStream.h)
//
// Menu.anm / CAG_Menu.anm binary formatı (2026-10-10 tersine mühendislik):
//
//   Header (40B): u32{0, dataSize, hdr2=433, hdr3} + u32 libid(0x1803ffff)
//                 + u32 0 + f32{0, -0, canvasW=640, canvasH=480}
//   @40:  u32 nodeCount
//   Node kayıtları (değişken): [u32 nameLen][name\0][u32 type][u32 arg]
//     type=1 → group/scene ("/MusicSoundWave/"), arg = sahne/meta id
//     type=3 → button binding (btnUp...), arg = pad kodu (ör. 98=up)
//     type=6 → command (QuitGame, NewSave...), arg = 0xFFFFFFFF
//   Sahne bölümü: u32{sizeA, numRecords, sizeB} + libid + ...
// ============================================================================

#include "rwcore.h"

enum Rt2dMaestroNodeType
{
    rwrt2dNODE_GROUP   = 1, ///< "/name/" sahne/grup düğümü
    rwrt2dNODE_BUTTON  = 3, ///< btnUp/btnCross... — arg = pad kodu
    rwrt2dNODE_COMMAND = 6  ///< QuitGame/NewSave... — arg = -1
};

#define rwrt2dMAX_NODES 64

struct Rt2dMaestroNode
{
    char     name[64];
    RwUInt32 type;  ///< Rt2dMaestroNodeType
    RwUInt32 arg;
};

struct Rt2dMaestroStreamInfo
{
    RwUInt32 field1;         ///< header[1] — veri boyutu benzeri
    RwUInt32 field2;         ///< header[2] — iki menüde de 433
    RwUInt32 field3;         ///< header[3]
    RwReal   canvasWidth;    ///< 640.0
    RwReal   canvasHeight;   ///< 480.0
    RwUInt32 nodeCount;      ///< parsed node sayısı (<= rwrt2dMAX_NODES)
    Rt2dMaestroNode nodes[rwrt2dMAX_NODES];
    RwUInt32 sceneSectionOffset; ///< anm başlangıcına göre sahne bölümü
    RwUInt32 sceneSizeA;     ///< sahne bölümü[0]
    RwUInt32 sceneNumRecords;///< sahne bölümü[1]
    RwUInt32 sceneSizeB;     ///< sahne bölümü[2]
};

/**
 * @brief .anm stream'ini parse eder (header + node tablosu + sahne başlığı).
 * @param pData Tam .anm payload (asset kaydı HARİÇ; stream'in kendisi).
 * @param len   Payload uzunluğu.
 * @param pInfo Çıktı bilgi yapısı.
 * @return TRUE başarılı; FALSE bozuk/eksik veri.
 */
RwBool Rt2dMaestro_Inspect(const void *pData, RwUInt32 len,
                           Rt2dMaestroStreamInfo *pInfo);

/**
 * @brief .txd/.dff asset kaydı içindeki gömülü .anm stream'ini bulur.
 *
 * RwStudio 0x0716 kayıt şeması: [metaLen][strLen][name][guid16]
 * [idNameLen]["rwID_2DMAESTRO\0"...]... kayıt sonu = metaLen field + metaLen;
 * stream orada başlar.
 *
 * @param pData  Tam asset chunk payload'u (0x0716 header'ı DAHİL).
 * @param len    Chunk uzunluğu (header hariç payload).
 * @param pOff   Çıktı: .anm stream'inin payload içindeki offset'i.
 * @param pLen   Çıktı: .anm stream uzunluğu.
 * @return TRUE bulundu.
 */
RwBool Rt2dMaestro_FindAnimInAsset(const void *pData, RwUInt32 len,
                                   RwUInt32 *pOff, RwUInt32 *pLen);

/**
 * @brief RWS dosyasındaki ilk gömülü .anm'i parse edip maestro kurar.
 *        Altyapı: main.cpp menü/harita fazlarındaki üç kopya 0x0716-walk
 *        + testler bu tek yoldan geçer (NULL = bulunamadı/bozuk).
 */
struct Rt2dMaestro;
Rt2dMaestro *Rt2dMaestro_LoadFromRwsFile(const char *path);

#endif // RT2DMAESTROSTREAM_H

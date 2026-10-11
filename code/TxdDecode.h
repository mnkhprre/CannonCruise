// ============================================================================
// CannonCruise - Gömülü TXD (d3d8 IMAGE) decode
//
// Binary veri kaynağı: MAINMENU.RWS 0x0716 asset'i, gömülü payload
// (embeddedType=0x23, libId=0x1803FFFF). Payload düzeni (Faz 4.6l dump):
//
//   [u16 textureCount][u16 deviceID]                     (4 bayt başlık)
//   repeat textureCount:
//     [u32 flag]                                         (texture başına, dosyada 1)
//     [id=0x18][len][ver=0x1803FFFF]  raster: nested [id=1][16B:
//         u32 width, u32 height, u32 depth(bpp), u32 pitch] + piksel
//         verisi (pitch*height bayt; 4-aligned)
//     [id=0x06][len][ver]  texture struct: nested [id=1][4B:
//         filterMode+addressing], [id=2][name], [id=2][maskName];
//         gövde kuyruğunda [id=3][0][ver] terminator
//
// Not: 0x18长 = 12(nested hdr) + 16 + piksel + pad; 0x06长 = 68+kısa isim
// (nested toplam + 12B terminator). Piksel verisi d3d8 RGBA (ilk texture:
// beyaz 0x00FFFFFF).
// ============================================================================

#ifndef CANNONCRUISE_TXDDECODE_H
#define CANNONCRUISE_TXDDECODE_H

#include "rwcore.h"

#define rwtxdMAX_TEXTURES 64
#define rwtxdMAX_NAME 32

/**
 * @struct RwTxdTexture
 * @brief Decode edilmiş tek bir d3d8 texture kaydı.
 */
struct RwTxdTexture {
    RwChar name[rwtxdMAX_NAME];     ///< nested id=2 (NUL sonlu)
    RwChar maskName[rwtxdMAX_NAME]; ///< nested id=2 (boş olabilir)
    RwUInt32 filterFlags;           ///< nested id=1 (filter+addressing)
    RwUInt32 width;                 ///< raster nested: width
    RwUInt32 height;                ///< raster nested: height
    RwUInt32 depth;                 ///< raster nested: bpp (32)
    RwUInt32 pitch;                 ///< raster nested: satır baytı
    const RwUInt8 *pPixels;         ///< piksel verisi (payload içi işaretçi)
    RwUInt32 pixelBytes;            ///< pitch*height
};

/**
 * @struct RwTxdImage
 * @brief Decode edilmiş texture dictionary (d3d8 IMAGE).
 */
struct RwTxdImage {
    RwUInt16 textureCount;
    RwUInt16 deviceId;
    RwUInt32 count;
    RwTxdTexture textures[rwtxdMAX_TEXTURES];
};

/**
 * @brief Gömülü TXD payload'ını decode eder.
 *
 * Chunk sınırları ve nested yapılar bayt bayt doğrulanır; bozuk veride
 * kısmi sonuç + FALSE döner.
 *
 * @param pPayload Gömülü payload (RwStudioAssetReadEmbedded çıktısı).
 * @param len      Payload boyutu (embeddedLength).
 * @param pOut     Çıktı (texture listesi + piksel işaretçileri).
 * @return TRUE ise başlık + tüm texture'lar başarıyla okundu.
 */
RwBool RwTxdDecodeImage(const RwUInt8 *pPayload, RwUInt32 len,
                        RwTxdImage *pOut);

/**
 * @brief Seviye doku paketi decode (VIKING01 chunk0 özel formatı).
 *
 * Kayıtlar: [06 11][02][len][libid]<isim> … tokenlar …
 * [01][0x10][libid]<w,h,depth,pitch> + piksel (pitch*h, 32bpp RGBA).
 * pPixels girdi tamponunu gösterir (upload bitene dek canlı tutulmalı).
 */
RwBool RwTxdDecodeLevelPack(const RwUInt8 *pData, RwUInt32 len,
                            RwTxdImage *pOut);

#endif // CANNONCRUISE_TXDDECODE_H

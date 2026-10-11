// ============================================================================
// CannonCruise - Rt2dMaestro .anm stream parser (Rt2dMaestroStream.cpp)
//
// Binary Fonksiyonları (format): Menu.anm / CAG_Menu.anm — tersine
// mühendislik ile çözüldü (bkz. başlık). CMenuBehavior::Update
// (FUN_00443800) bu stream'i Rt2dMaestroCreate'e veriyordu.
// ============================================================================

#include "Rt2dMaestroStream.h"
#include "Rt2dMaestro.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// ---------------------------------------------------------------------------
// Rt2dMaestro_Inspect — header + node tablosu + sahne bölümü başlığı
// ---------------------------------------------------------------------------
RwBool Rt2dMaestro_Inspect(const void *pData, RwUInt32 len,
                           Rt2dMaestroStreamInfo *pInfo)
{
    if (pData == nullptr || pInfo == nullptr || len < 44) {
        return FALSE;
    }
    const RwUInt8 *p = static_cast<const RwUInt8 *>(pData);
    std::memset(pInfo, 0, sizeof(*pInfo));

    // Header (40B): u32{0, field1, field2, field3} + libid + 0 + 4×f32
    RwUInt32 hdr[4];
    std::memcpy(hdr, p, 16);
    if (hdr[0] != 0) {
        return FALSE; // bilinen stream'ler hdr[0]=0
    }
    pInfo->field1 = hdr[1];
    pInfo->field2 = hdr[2];
    pInfo->field3 = hdr[3];

    RwUInt32 libId = 0;
    std::memcpy(&libId, p + 16, 4);
    if (libId != 0x1803FFFF) {
        return FALSE; // RWS library id imzası
    }
    std::memcpy(&pInfo->canvasWidth, p + 32, 4);
    std::memcpy(&pInfo->canvasHeight, p + 36, 4);

    // Node tablosu
    RwUInt32 nodeCount = 0;
    std::memcpy(&nodeCount, p + 40, 4);
    if (nodeCount > rwrt2dMAX_NODES) {
        return FALSE; // bilinen menülerde max 44
    }
    RwUInt32 off = 44;
    for (RwUInt32 i = 0; i < nodeCount; ++i) {
        if (off + 4 > len) {
            return FALSE;
        }
        RwUInt32 nameLen = 0;
        std::memcpy(&nameLen, p + off, 4);
        off += 4;
        if (nameLen == 0 || nameLen > 63 || off + nameLen + 8 > len) {
            return FALSE;
        }
        Rt2dMaestroNode &node = pInfo->nodes[i];
        std::memcpy(node.name, p + off, nameLen);
        node.name[nameLen] = '\0';
        // 0xbf dolgu temizle (RwStudio string pad)
        for (RwUInt32 c = 0; c < nameLen; ++c) {
            if (node.name[c] == '\x00') {
                node.name[c] = '\0';
                break;
            }
        }
        off += nameLen;
        std::memcpy(&node.type, p + off, 4);
        std::memcpy(&node.arg, p + off + 4, 4);
        off += 8;
    }
    pInfo->nodeCount = nodeCount;

    // Sahne bölümü: [u32 sizeA][u32 numRecords][u32 sizeB]
    pInfo->sceneSectionOffset = off;
    if (off + 12 <= len) {
        std::memcpy(&pInfo->sceneSizeA, p + off, 4);
        std::memcpy(&pInfo->sceneNumRecords, p + off + 4, 4);
        std::memcpy(&pInfo->sceneSizeB, p + off + 8, 4);
    }
    return TRUE;
}

// ---------------------------------------------------------------------------
// Rt2dMaestro_FindAnimInAsset — 0x0716 kayıt payload'undan .anm bul
// Kayıt: [metaLen][strLen][name...][guid16][idNameLen]["rwID_2DMAESTRO"...]
// ---------------------------------------------------------------------------
RwBool Rt2dMaestro_FindAnimInAsset(const void *pData, RwUInt32 len,
                                   RwUInt32 *pOff, RwUInt32 *pLen)
{
    if (pData == nullptr || pOff == nullptr || pLen == nullptr) {
        return FALSE;
    }
    const RwUInt8 *p = static_cast<const RwUInt8 *>(pData);
    static const char kIdName[] = "rwID_2DMAESTRO";
    const RwUInt32 kIdNameLen = 15; // null dahil

    // idName dizesini tara
    for (RwUInt32 i = 0; i + kIdNameLen < len; ++i) {
        if (std::memcmp(p + i, kIdName, kIdNameLen) != 0) {
            continue;
        }
        // idNameLen field = i-4; guid16 = i-4-16. metaLen field name uzunluğu
        // değişken olduğu için geriye taranır: metaLen@m kapsıyorsa ve
        // m+4'teki nameLen mantıklıysa kayıttır.
        if (i < 24) {
            continue;
        }
        const RwUInt32 guidStart = i - 4 - 16;
        for (RwInt32 m = (RwInt32)guidStart; m >= 0; m -= 4) {
            RwUInt32 metaLen = 0;
            std::memcpy(&metaLen, p + m, 4);
            if (metaLen <= guidStart - (RwUInt32)m + 4 ||
                (RwUInt32)m + metaLen > len) {
                continue;
            }
            RwUInt32 nameLen = 0;
            std::memcpy(&nameLen, p + m + 4, 4);
            if (nameLen < 4 || nameLen > 63 || (RwUInt32)m + 8 + nameLen > len) {
                continue;
            }
            // isim yazdırılabilir (0xbf pad hariç)
            RwBool printable = TRUE;
            for (RwUInt32 c = 0; c < nameLen; ++c) {
                const RwUInt8 ch = p[(RwUInt32)m + 8 + c];
                if (ch != 0 && ch != 0xBF && (ch < 32 || ch >= 127)) {
                    printable = FALSE;
                    break;
                }
            }
            if (printable == FALSE) {
                continue;
            }
            *pOff = (RwUInt32)m + metaLen;
            if (*pOff >= len) {
                return FALSE;
            }
            *pLen = len - *pOff;
            return TRUE;
        }
    }
    return FALSE;
}

Rt2dMaestro *Rt2dMaestro_LoadFromRwsFile(const char *path)
{
    if (path == 0) {
        return 0;
    }
    FILE *f = std::fopen(path, "rb");
    if (f == 0) {
        return 0;
    }
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    Rt2dMaestro *out = 0;
    if (size > 0 && size <= 64L * 1024 * 1024) {
        RwUInt8 buf[12];
        // Chunk yürüyüşü akışla (belleğe tüm dosya alınmaz).
        // Not: 0x0716 payload'u için rasgele erişim gerekir → iki aşama:
        // önce aday konumu bul, sonra payload'u oku.
        long animOff = -1;
        RwUInt32 animLen = 0;
        long off = 0;
        while (off + 12 <= size) {
            std::fseek(f, off, SEEK_SET);
            if (std::fread(buf, 1, 12, f) != 12) {
                break;
            }
            RwUInt32 ctype = 0, clen = 0;
            std::memcpy(&ctype, buf, 4);
            std::memcpy(&clen, buf + 4, 4);
            if (clen < 12 || off + 12 + (long)clen > size) {
                break;
            }
            if (ctype == 0x0716) {
                RwUInt8 *payload =
                    static_cast<RwUInt8 *>(std::malloc(clen));
                if (payload != 0) {
                    std::fseek(f, off + 12, SEEK_SET);
                    if (std::fread(payload, 1, clen, f) == clen) {
                        RwUInt32 aoff = 0, alen = 0;
                        if (Rt2dMaestro_FindAnimInAsset(payload, clen, &aoff,
                                                        &alen) == TRUE) {
                            animOff = off + 12 + (long)aoff;
                            animLen = alen;
                        }
                    }
                    std::free(payload);
                }
                if (animOff >= 0) {
                    break;
                }
            }
            off += 12 + (long)clen;
        }
        if (animOff >= 0 && animLen > 0) {
            RwUInt8 *abuf =
                static_cast<RwUInt8 *>(std::malloc(animLen));
            if (abuf != 0) {
                std::fseek(f, animOff, SEEK_SET);
                if (std::fread(abuf, 1, animLen, f) == animLen) {
                    out = Rt2dMaestroCreateFromMemory(abuf, animLen);
                }
                std::free(abuf);
            }
        }
    }
    std::fclose(f);
    return out;
}

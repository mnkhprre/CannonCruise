// ============================================================================
// CannonCruise - TXD (d3d8 IMAGE) decode implementation
// ============================================================================

#include "TxdDecode.h"

#include <cstring>

namespace {

struct Cursor {
    const RwUInt8 *pData;
    RwUInt32 len;
    RwUInt32 pos;
};

RwBool ReadU32(Cursor *pCur, RwUInt32 *pOut) {
    if (pCur->pos + 4 > pCur->len) {
        return FALSE;
    }
    std::memcpy(pOut, pCur->pData + pCur->pos, 4);
    pCur->pos += 4;
    return TRUE;
}

RwBool ReadU16(Cursor *pCur, RwUInt16 *pOut) {
    if (pCur->pos + 2 > pCur->len) {
        return FALSE;
    }
    std::memcpy(pOut, pCur->pData + pCur->pos, 2);
    pCur->pos += 2;
    return TRUE;
}

// [id][len][ver] başlığı; ver != 0x1803FFFF veya taşma ise FALSE
RwBool ReadChunkHeader(Cursor *pCur, RwUInt32 *pId, RwUInt32 *pLen) {
    RwUInt32 ver = 0;
    if (!ReadU32(pCur, pId) || !ReadU32(pCur, pLen) || !ReadU32(pCur, &ver)) {
        return FALSE;
    }
    if (ver != 0x1803FFFF || *pLen > pCur->len - pCur->pos) {
        return FALSE;
    }
    return TRUE;
}

void SkipAlign4(Cursor *pCur) {
    while ((pCur->pos & 3) != 0 && pCur->pos < pCur->len) {
        ++pCur->pos;
    }
}

// String chunk gövdesi: NUL sonlu, kalan bayt 0/0xBF pad — NUL'a kadar kopya
void ReadPaddedString(const RwUInt8 *pBody, RwUInt32 bodyLen, RwChar *pOut,
                      RwUInt32 maxOut) {
    RwUInt32 n = 0;
    for (; n < bodyLen && pBody[n] != '\0' && n < maxOut - 1; ++n) {
        pOut[n] = static_cast<RwChar>(pBody[n]);
    }
    pOut[n] = '\0';
}

} // namespace

RwBool RwTxdDecodeImage(const RwUInt8 *pPayload, RwUInt32 len,
                        RwTxdImage *pOut) {
    if (!pPayload || !pOut || len < 8) {
        return FALSE;
    }
    std::memset(pOut, 0, sizeof(*pOut));

    Cursor cur = {pPayload, len, 0};
    if (!ReadU16(&cur, &pOut->textureCount) ||
        !ReadU16(&cur, &pOut->deviceId)) {
        return FALSE;
    }

    for (RwUInt32 t = 0; t < pOut->textureCount; ++t) {
        if (pOut->count >= rwtxdMAX_TEXTURES) {
            return FALSE;
        }
        RwTxdTexture *pTex = &pOut->textures[pOut->count];

        // ---- texture başına flag ----
        RwUInt32 texFlag = 0;
        if (!ReadU32(&cur, &texFlag)) {
            return FALSE;
        }
        (void)texFlag; // anlamı bilinmiyor (dosyada 1)

        // ---- 0x18 raster ----
        RwUInt32 id = 0, clen = 0;
        if (!ReadChunkHeader(&cur, &id, &clen) || id != 0x18) {
            return FALSE;
        }
        {
            Cursor body = {cur.pData, cur.pos + clen, cur.pos};
            RwUInt32 nid = 0, nlen = 0;
            if (!ReadChunkHeader(&body, &nid, &nlen) || nid != 1 ||
                nlen != 16) {
                return FALSE;
            }
            if (!ReadU32(&body, &pTex->width) ||
                !ReadU32(&body, &pTex->height) ||
                !ReadU32(&body, &pTex->depth) ||
                !ReadU32(&body, &pTex->pitch)) {
                return FALSE;
            }
            pTex->pixelBytes = pTex->pitch * pTex->height;
            if (pTex->pixelBytes > body.len - body.pos) {
                return FALSE;
            }
            pTex->pPixels = body.pData + body.pos;
        }
        cur.pos += clen;
        SkipAlign4(&cur);

        // ---- 0x06 texture struct ----
        if (!ReadChunkHeader(&cur, &id, &clen) || id != 0x06) {
            return FALSE;
        }
        {
            Cursor body = {cur.pData, cur.pos + clen, cur.pos};
            RwUInt32 nid = 0, nlen = 0;
            // nested id=1: filterFlags
            if (!ReadChunkHeader(&body, &nid, &nlen) || nid != 1 ||
                nlen != 4 || !ReadU32(&body, &pTex->filterFlags)) {
                return FALSE;
            }
            // nested id=2: name
            if (!ReadChunkHeader(&body, &nid, &nlen) || nid != 2 ||
                nlen > body.len - body.pos) {
                return FALSE;
            }
            ReadPaddedString(body.pData + body.pos, nlen, pTex->name,
                             rwtxdMAX_NAME);
            body.pos += nlen;
            // nested id=2: maskName
            if (!ReadChunkHeader(&body, &nid, &nlen) || nid != 2 ||
                nlen > body.len - body.pos) {
                return FALSE;
            }
            ReadPaddedString(body.pData + body.pos, nlen, pTex->maskName,
                             rwtxdMAX_NAME);
            // gövde kuyruğu: [id=3][0][ver] terminator — kapalı alan atlanır
        }
        cur.pos += clen;
        SkipAlign4(&cur);
        ++pOut->count;
    }
    return pOut->count == pOut->textureCount;
}

namespace {

// 2'nin kuvveti doku boyutu mu (8..512)?
bool IsPow2Size(RwUInt32 v) {
    return v == 8 || v == 16 || v == 32 || v == 64 || v == 128 ||
           v == 256 || v == 512;
}

// Yazdırılabilir doku adı mı ([A-Za-z][A-Za-z0-9_]{1,..}, NUL sonlu)?
bool ValidLevelName(const RwUInt8 *p, RwUInt32 len) {
    if (len == 0 || len > 48) {
        return false;
    }
    if (!((p[0] >= 'A' && p[0] <= 'Z') ||
          (p[0] >= 'a' && p[0] <= 'z'))) {
        return false;
    }
    RwUInt32 i = 0;
    for (; i < len; ++i) {
        const RwUInt8 c = p[i];
        if (c == 0) {
            break;
        }
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '_')) {
            return false;
        }
    }
    return i > 1 && i < len;
}

} // namespace

RwBool RwTxdDecodeLevelPack(const RwUInt8 *pData, RwUInt32 len,
                            RwTxdImage *pOut) {
    if (!pData || !pOut || len < 32) {
        return FALSE;
    }
    std::memset(pOut, 0, sizeof(*pOut));
    // İsim kaydı imzası: 06 11 00 00 02 00 00 00 len(<=48) libid
    static const RwUInt8 kSig[8] = {0x06, 0x11, 0x00, 0x00,
                                    0x02, 0x00, 0x00, 0x00};
    static const RwUInt8 kLib[4] = {0xFF, 0xFF, 0x03, 0x18};
    static const RwUInt8 kDims[12] = {0x01, 0x00, 0x00, 0x00,
                                      0x10, 0x00, 0x00, 0x00,
                                      0xFF, 0xFF, 0x03, 0x18};
    for (RwUInt32 o = 0; o + 16 < len; ++o) {
        if (std::memcmp(pData + o, kSig, 8) != 0) {
            continue;
        }
        RwUInt32 nlen = 0;
        std::memcpy(&nlen, pData + o + 8, 4);
        if (nlen == 0 || nlen > 48 || o + 16 + nlen > len) {
            continue;
        }
        if (std::memcmp(pData + o + 12, kLib, 4) != 0) {
            continue;
        }
        const RwUInt8 *np = pData + o + 16;
        if (!ValidLevelName(np, nlen)) {
            continue;
        }
        // Boyut kaydı: 30KB pencerede ilk geçerli dims imzası.
        RwUInt32 base = o + 16 + nlen;
        for (RwUInt32 q = base; q + 28 < len && q < base + 30000; ++q) {
            if (std::memcmp(pData + q, kDims, 12) != 0) {
                continue;
            }
            RwUInt32 w = 0, h = 0, dep = 0, pi = 0;
            std::memcpy(&w, pData + q + 12, 4);
            std::memcpy(&h, pData + q + 16, 4);
            std::memcpy(&dep, pData + q + 20, 4);
            std::memcpy(&pi, pData + q + 24, 4);
            if (!IsPow2Size(w) || !IsPow2Size(h) || dep != 32) {
                continue;
            }
            if (pi < w * 4 || pi > w * 4 + 64) {
                continue;
            }
            const RwUInt32 px = q + 28;
            if (px + pi * h > len) {
                continue;
            }
            if (pOut->count >= rwtxdMAX_TEXTURES) {
                return (pOut->count > 0) ? TRUE : FALSE;
            }
            RwTxdTexture *t = &pOut->textures[pOut->count];
            RwUInt32 ni = 0;
            for (; ni < nlen && np[ni] != 0 &&
                   ni + 1 < (RwUInt32)rwtxdMAX_NAME;
                 ++ni) {
                t->name[ni] = static_cast<RwChar>(np[ni]);
            }
            t->name[ni] = 0;
            t->width = w;
            t->height = h;
            t->depth = dep;
            t->pitch = pi;
            t->pPixels = pData + px;
            t->pixelBytes = pi * h;
            ++pOut->count;
            break; // bu isim için ilk geçerli dims
        }
    }
    pOut->textureCount = (RwUInt16)pOut->count;
    return (pOut->count > 0) ? TRUE : FALSE;
}

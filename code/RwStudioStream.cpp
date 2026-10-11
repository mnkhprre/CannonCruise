// ============================================================================
// CannonCruise - rwstudio Konteyner Okuyucu (RwStudioStream.cpp)
// Original path: D:\Projects\CannonCruisePC\code\RwStudioStream.cpp
//
// Format kanıtı: Streams/MAINMENU.RWS + GLOBAL.RWS + VIKING01.RWS hexdump
// (Faz 4.6e). Örnek — MAINMENU 0x0716 #0 (txd):
//   [12B RW başlık 0x0716/libid 0x1802FFFF]
//   [u32 metaLen=0xC8][typeName][GUID16][rwIdName][sourcePath]
//   [u32 4][0xBF×3]                       <- meta kuyruğu (metaLen içinde)
//   [u32 resource][u32 va]                 <- rwstudio iç alanlar
//   [u32 embType=0x23][u32 embLen][u32 libid 0x1803FFFF]
//   [gömülü payload: embLen-12 bayt]
//     payload: i16 dokuSayısı, i16 device=1 (d3d8), IMAGE chunk'ları...
// embLen, embType alanından chunk sonuna kadar (12B başlık dahil):
// metaLen+20+embLen == chunkLength doğrulandı.
// Non-embed (WorldGeometry/.dff, StaticCollision/.col): embType=0;
// .col verisi chunk içinde inline (CPhysicsResource::Load bu alanı okur).
// ============================================================================

#include "RwStudioStream.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

struct RwStudioFile {
    RwStream *pStream;
    RwUInt32 pendingPayload;  // gömülü payload bayt sayısı (okunmadıysa)
    RwUInt32 embedPayloadPos; // gömülü payload'un dosya konumu
    RwUInt32 chunkEndPos;     // mevcut chunk sonu (mutlak)
    RwUInt32 classCount;
    RwChar classNames[rwstudioMAX_CLASSES][48];
    RwBool hasObject;         // son NextAsset 0x0704 mü?
    RwStudioObject object;    // son nesne şeması (Faz 4.6f)
};

namespace
{

// uzunluk önekli string: [u32 len][len bayt; '\0'+0xBF doldurma, 4 hizalı]
RwBool ReadLenString(RwStream *pStream, RwChar *pOut, RwUInt32 maxOut) {
    RwUInt32 len = 0;
    if (RwStreamRead(pStream, &len, 4) != 4 || len == 0 || len > 512) {
        return FALSE;
    }
    RwChar buf[512];
    if (RwStreamRead(pStream, buf, len) != len) {
        return FALSE;
    }
    RwUInt32 n = 0;
    while (n < len && n < maxOut - 1 && buf[n] != '\0') {
        pOut[n] = buf[n];
        ++n;
    }
    pOut[n] = '\0';
    return TRUE;
}

// manifest string'i: null-terminated + 0xBF doldurma (4 hizalı), uzunluk yok
RwBool ReadPadString(RwStream *pStream, RwChar *pOut, RwUInt32 maxOut,
                     RwUInt32 *pConsumed) {
    RwChar buf[64];
    RwUInt32 n = 0;
    while (n < sizeof(buf)) {
        if (RwStreamRead(pStream, &buf[n], 1) != 1) {
            return FALSE;
        }
        if (buf[n] == '\0') {
            ++n;
            break;
        }
        ++n;
    }
    while (n % 4 != 0) {
        RwUInt8 pad = 0;
        if (RwStreamRead(pStream, &pad, 1) != 1) {
            return FALSE;
        }
        if (pad != 0xBF && pad != 0x00) {
            return FALSE;
        }
        ++n;
    }
    RwUInt32 copy = n < maxOut - 1 ? n : maxOut - 1;
    RwUInt32 i = 0;
    for (; i < copy && buf[i] != '\0'; ++i) {
        pOut[i] = buf[i];
    }
    pOut[i] = '\0';
    if (pConsumed) {
        *pConsumed = n;
    }
    return TRUE;
}

} // namespace

RwStudioFile *RwStudioFileOpen(const RwChar *pPath) {
    if (!pPath) {
        return 0;
    }
    RwStream *pStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, pPath);
    if (!pStream) {
        return 0;
    }
    RwStudioFile *pFile = new RwStudioFile();
    pFile->pStream = pStream;
    pFile->pendingPayload = 0;
    pFile->embedPayloadPos = 0;
    pFile->chunkEndPos = 0;
    pFile->classCount = 0;
    pFile->hasObject = FALSE;
    return pFile;
}

void RwStudioFileClose(RwStudioFile *pFile) {
    if (!pFile) {
        return;
    }
    if (pFile->pStream) {
        RwStreamClose(pFile->pStream, 0);
    }
    delete pFile;
}

RwBool RwStudioFileNextAsset(RwStudioFile *pFile, RwStudioAsset *pAsset) {
    if (!pFile || !pAsset) {
        return FALSE;
    }
    // önceki asset'in gömülü payload'u okunmadıysa chunk sonuna atla
    if (pFile->pendingPayload > 0) {
        RwStreamSeekAbs(pFile->pStream, pFile->chunkEndPos);
        pFile->pendingPayload = 0;
    }
    std::memset(pAsset, 0, sizeof(*pAsset));
    pFile->hasObject = FALSE;

    // chunk başlangıcı + bitişi: seek tabanlı hizalama (format varyantları
    // byte-sayımı yerine mutlak konumla güvenli atlanır)
    const RwUInt32 chunkStart = RwStreamTell(pFile->pStream);

    RwChunkHeaderInfo header;
    std::memset(&header, 0, sizeof(header));
    if (!RwStreamReadChunkHeaderInfo(pFile->pStream, &header)) {
        if (std::getenv("RWSTUDIO_DEBUG")) {
            std::fprintf(stderr, "NextAsset EOF @tell=%u\n",
                         RwStreamTell(pFile->pStream));
        }
        return FALSE; // EOF
    }
    if (header.type == 0) {
        return FALSE;
    }
    pAsset->chunkType = header.type;
    pAsset->chunkLength = header.length;
    pFile->chunkEndPos = chunkStart + 12 + header.length;

    if (header.type == rwstudioID_ASSETRECORD) {
        RwUInt32 metaLen = 0;
        if (RwStreamRead(pFile->pStream, &metaLen, 4) != 4 || metaLen < 8 ||
            metaLen > 4096) {
            return FALSE;
        }
        if (!ReadLenString(pFile->pStream, pAsset->typeName,
                           rwstudioMAX_NAME)) {
            return FALSE;
        }
        if (RwStreamRead(pFile->pStream, pAsset->guid, 16) != 16) {
            return FALSE;
        }
        if (!ReadLenString(pFile->pStream, pAsset->rwIdName,
                           rwstudioMAX_NAME)) {
            return FALSE;
        }
        if (!ReadLenString(pFile->pStream, pAsset->sourcePath,
                           rwstudioMAX_PATH)) {
            return FALSE;
        }
        // meta kuyruğu: standart asset'lerde [u32 4][0xBF×3+0x00]; animasyon
        // (rwID_2DMAESTRO) gibi varyantlarda bu YOK — dördüncü string meta'yı
        // tamamlar. Tolerant kural: sonraki u32==4 ise pad4 + standart alan;
        // değilse resource'un kendisi (va/A/B kaymaz — iki dalda da toplam
        // metaLen+16, embed'de +4 libid).
        RwUInt32 tail = 0;
        if (RwStreamRead(pFile->pStream, &tail, 4) != 4) {
            return FALSE;
        }
        if (tail == 4) {
            RwUInt8 pad[4];
            if (RwStreamRead(pFile->pStream, pad, 4) != 4) {
                return FALSE;
            }
        }
        // [u32 resource][u32 va][u32 A][u32 B] (+ embed ise [u32 libid])
        // — txd: [res][va][0x23][embLen][libid][payload]
        //      wbID_Texture: [res][len][pngPath]; 2DMAESTRO/PPB: kendi
        //      stream'leri. Embed TANIMI kimlik ile: A/B varyant yorumu
        //      gerekmez; diğerleri chunk sonuna seek ile atlanır.
        RwUInt32 resource = 0, va = 0;
        if (RwStreamRead(pFile->pStream, &resource, 4) != 4 ||
            RwStreamRead(pFile->pStream, &va, 4) != 4) {
            RwStreamSeekAbs(pFile->pStream, pFile->chunkEndPos);
            return TRUE;
        }
        (void)resource;
        (void)va;
        RwUInt32 embA = 0, embB = 0;
        if (RwStreamRead(pFile->pStream, &embA, 4) != 4 ||
            RwStreamRead(pFile->pStream, &embB, 4) != 4) {
            RwStreamSeekAbs(pFile->pStream, pFile->chunkEndPos);
            return TRUE;
        }
        if (embB >= 12 && metaLen + 20 + embB == header.length) {
            pAsset->embeddedType = embA;
            pAsset->embeddedLength = embB;
            if (RwStreamRead(pFile->pStream, &pAsset->embeddedLibId, 4) !=
                4) {
                RwStreamSeekAbs(pFile->pStream, pFile->chunkEndPos);
                return TRUE;
            }
            // embB klasik RW konvansiyonu: payload boyutu (12B başlık hariç)
            pFile->embedPayloadPos = RwStreamTell(pFile->pStream);
            pFile->pendingPayload = embB;
        } else {
            // embed yok — resource payload'u chunk sonuna kadar (örn. .col):
            // kaydet + pendingPayload ile OKUNMAZSA sonraki NextAsset atlar
            const RwUInt32 pos = RwStreamTell(pFile->pStream);
            pFile->embedPayloadPos = pos;
            if (pFile->chunkEndPos > pos) {
                pAsset->resourceLength = pFile->chunkEndPos - pos;
                pFile->pendingPayload = pAsset->resourceLength;
            } else {
                RwStreamSeekAbs(pFile->pStream, pFile->chunkEndPos);
            }
        }
        return TRUE;
    }

    if (header.type == rwstudioID_MANIFEST) {
        // [u32 count][entry: (string+0xBF pad) + u32 sürüm]...[kuyruk]
        // — sayım/entry hatalarında bile chunk sonuna seek ile hizalanır
        RwUInt32 count = 0;
        if (RwStreamRead(pFile->pStream, &count, 4) == 4 && count <= 256) {
            for (RwUInt32 i = 0; i < count; ++i) {
                RwChar name[48];
                if (!ReadPadString(pFile->pStream, name, sizeof(name), 0)) {
                    break;
                }
                RwUInt32 version = 0;
                if (RwStreamRead(pFile->pStream, &version, 4) != 4) {
                    break;
                }
                if (i < rwstudioMAX_CLASSES) {
                    std::strncpy(pFile->classNames[i], name,
                                 sizeof(pFile->classNames[0]) - 1);
                    pFile->classNames[i][sizeof(pFile->classNames[0]) - 1] =
                        '\0';
                }
            }
            pFile->classCount =
                count < rwstudioMAX_CLASSES ? count : rwstudioMAX_CLASSES;
        }
        RwStreamSeekAbs(pFile->pStream, pFile->chunkEndPos);
        return TRUE;
    }

    if (header.type == rwstudioID_OBJECTRECORD) {
        // 0x0704: [u32 field0] + segment/property dizisi (bkz. header
        // şeması). Chunk sonu seek ile garanti altına alınır; parse
        // hatası sessizce yarım nesne bırakır.
        RwStudioObject &obj = pFile->object;
        std::memset(&obj, 0, sizeof(obj));
        if (RwStreamRead(pFile->pStream, &obj.objectIndex, 4) != 4) {
            RwStreamSeekAbs(pFile->pStream, pFile->chunkEndPos);
            return TRUE;
        }
        RwStudioBehaviour *pCur = 0;
        // 0x80 davranışları: ilk behaviour'da ad tekrarı (0x20+GUID sonrası),
        // sonraki 0x80 = YENI behaviour (ad tek segment — GUID yok).
        // Durum: afterStart = 0x20 sonrası (GUID beklemede);
        //        afterGuid = GUID sonrası (ad tekrarı beklemede).
        int state = 0;
        while (RwStreamTell(pFile->pStream) + 8 <= pFile->chunkEndPos) {
            RwUInt32 x = 0;
            if (RwStreamRead(pFile->pStream, &x, 4) != 4) {
                break;
            }
            if (x == 0x1C || x == 0x18) {
                // segment: [segLen][flags][içerik]
                RwUInt32 flags = 0;
                if (RwStreamRead(pFile->pStream, &flags, 4) != 4) {
                    break;
                }
                if (flags == 0x20000000) {
                    if (obj.behaviourCount >= rwstudioMAX_BEHAVIOURS) {
                        break;
                    }
                    pCur = &obj.behaviours[obj.behaviourCount++];
                    std::memset(pCur, 0, sizeof(*pCur));
                    ReadPadString(pFile->pStream, pCur->className,
                                  sizeof(pCur->className), 0);
                    state = 1; // GUID bekle
                } else if (flags == 0x40000000) {
                    if (pCur) {
                        RwStreamRead(pFile->pStream, pCur->guid, 16);
                    }
                    state = 2; // ad tekrarı bekle
                } else if (flags == 0x80000000) {
                    RwChar name[48];
                    ReadPadString(pFile->pStream, name, sizeof(name), 0);
                    if (state == 2 && pCur &&
                        std::strcmp(name, pCur->className) == 0) {
                        // ad tekrarı — ad zaten 0x20'de okundu; doğrula
                        state = 3;
                    } else {
                        // yeni behaviour (farklı ad — GUID'suz tekrar kaydı)
                        if (obj.behaviourCount >= rwstudioMAX_BEHAVIOURS) {
                            break;
                        }
                        pCur = &obj.behaviours[obj.behaviourCount++];
                        std::memset(pCur, 0, sizeof(*pCur));
                        std::strncpy(pCur->className, name,
                                     sizeof(pCur->className) - 1);
                        state = 3;
                    }
                } else if (flags == 0x00000000) {
                    // flags=0: string segment (ör. ses entity adı) VEYA 16B
                    // blob (GUID — ör. CSystemCommands alt segmentleri).
                    // içerik uzunluğu kesin: x-8; ReadPadString blob'ta
                    // aşırı okuyup stream'i kaydırırdı — tam uzunluk tüket.
                    RwUInt32 contentLen = x >= 8 ? x - 8 : 0;
                    RwChar buf[64];
                    if (contentLen > sizeof(buf)) {
                        // bilinmeyen uzun blob — chunk sonuna seek
                        RwStreamSeekAbs(pFile->pStream, pFile->chunkEndPos);
                        break;
                    }
                    if (contentLen > 0 &&
                        RwStreamRead(pFile->pStream, buf, contentLen) !=
                            contentLen) {
                        break;
                    }
                    // tespit: ilk bayt yazdırılabilir → string; değilse blob
                    if (pCur && contentLen > 0 && buf[0] >= 0x20 &&
                        buf[0] < 0x7F) {
                        RwUInt32 i = 0;
                        for (; i + 1 < sizeof(pCur->name) && i < contentLen &&
                                             buf[i] != '\0';
                             ++i) {
                            pCur->name[i] = buf[i];
                        }
                        pCur->name[i] = '\0';
                    }
                } else {
                    break; // bilinmeyen segment — chunk sonuna seek
                }
            } else if (x == 12 || x == 20) {
                // property: [size][id][size-8 değer] — kapasite yoksa da
                // baytları tüket (hizayı koru)
                RwUInt32 propId = 0;
                if (RwStreamRead(pFile->pStream, &propId, 4) != 4) {
                    break;
                }
                RwUInt8 valueBuf[16];
                RwUInt32 valueLen = x - 8;
                std::memset(valueBuf, 0, sizeof(valueBuf));
                if (RwStreamRead(pFile->pStream, valueBuf, valueLen) !=
                    valueLen) {
                    break;
                }
                if (pCur && pCur->propCount < rwstudioMAX_PROPS) {
                    RwStudioProperty &prop = pCur->props[pCur->propCount];
                    prop.id = propId;
                    prop.size = x;
                    std::memcpy(prop.value, valueBuf, valueLen);
                    ++pCur->propCount;
                }
            } else {
                break; // beklenmeyen alan — hizayı koru
            }
        }
        pFile->hasObject = TRUE;
        RwStreamSeekAbs(pFile->pStream, pFile->chunkEndPos);
        return TRUE;
    }

    // bilinmeyen türler: chunk sonuna seek
    RwStreamSeekAbs(pFile->pStream, pFile->chunkEndPos);
    return TRUE;
}

RwUInt32 RwStudioAssetReadEmbedded(RwStudioFile *pFile,
                                   const RwStudioAsset *pAsset, void *pBuf,
                                   RwUInt32 maxLen) {
    if (!pFile || !pAsset || !pBuf || pFile->pendingPayload == 0) {
        return 0;
    }
    // mutlak konum + chunk sonu seek: hizа her koşulda korunur
    RwStreamSeekAbs(pFile->pStream, pFile->embedPayloadPos);
    RwUInt32 toRead = pFile->pendingPayload;
    if (toRead > maxLen) {
        toRead = maxLen;
    }
    RwUInt32 got = RwStreamRead(pFile->pStream, pBuf, toRead);
    RwStreamSeekAbs(pFile->pStream, pFile->chunkEndPos);
    pFile->pendingPayload = 0;
    return got;
}

RwUInt32 RwStudioAssetReadResource(RwStudioFile *pFile,
                                   const RwStudioAsset *pAsset, void *pBuf,
                                   RwUInt32 maxLen) {
    if (!pFile || !pAsset || !pBuf || pFile->pendingPayload == 0 ||
        pAsset->resourceLength == 0 || pAsset->embeddedType != 0) {
        return 0;
    }
    // embed payload mekanizması aynı alanları paylaşır (pos + pending)
    return RwStudioAssetReadEmbedded(pFile, pAsset, pBuf, maxLen);
}

RwUInt32 RwStudioFileGetClassCount(const RwStudioFile *pFile) {
    return pFile ? pFile->classCount : 0;
}

const RwChar *RwStudioFileGetClassName(const RwStudioFile *pFile,
                                       RwUInt32 index) {
    if (!pFile || index >= pFile->classCount) {
        return "";
    }
    return pFile->classNames[index];
}

const RwStudioObject *RwStudioFileGetObject(const RwStudioFile *pFile) {
    if (!pFile || !pFile->hasObject) {
        return 0;
    }
    return &pFile->object;
}

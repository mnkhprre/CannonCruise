// ============================================================================
// CannonCruise - WorldGeometry decode + clump instantiate (WorldGeometry.cpp)
// Faz 4.6o: binary FUN_004c9b70 (ATOMIC döngüsü) + VERİ hexdump çözümü.
// Prefix [len][libid] + STRUCT/GEOMETRY/EXT + ATOMIC×N (gömülü geometry).
// ============================================================================

#include "WorldGeometry.h"

#include <rw.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

const RwUInt32 kLibraryIdRW36 = 0x1803FFFF;
const RwUInt32 kChunkStruct = 0x01;
const RwUInt32 kChunkGeometry = 0x0F;
const RwUInt32 kChunkAtomic = 0x14;

// Son CreateClump'un dünya bbox'ı — auto-framing (4.6q)
WorldGeometryBounds s_Bounds;

struct Cursor {
    const RwUInt8 *pBuf;
    RwUInt32 len;
    RwUInt32 pos;
};

inline RwUInt32 ReadU32(const Cursor &c) {
    RwUInt32 v;
    std::memcpy(&v, c.pBuf + c.pos, 4);
    return v;
}

inline RwBool Have(const Cursor &c, RwUInt32 n) {
    return (c.pos + n <= c.len) ? TRUE : FALSE;
}

inline RwUInt32 ReadU32At(const RwUInt8 *pBuf, RwUInt32 pos) {
    RwUInt32 v;
    std::memcpy(&v, pBuf + pos, 4);
    return v;
}

/// [type][len][libid] 12B başlık; libid 0x1803FFFF ve tip beklendi.
inline RwBool ReadHeader(Cursor &c, RwUInt32 expectType, RwUInt32 *pLen) {
    if (!Have(c, 12)) {
        return FALSE;
    }
    const RwUInt32 type = ReadU32(c);
    c.pos += 4;
    const RwUInt32 len = ReadU32(c);
    c.pos += 4;
    const RwUInt32 libid = ReadU32(c);
    c.pos += 4;
    if (type != expectType || libid != kLibraryIdRW36) {
        return FALSE;
    }
    if (!Have(c, len)) {
        return FALSE;
    }
    *pLen = len;
    return TRUE;
}

/**
 * @brief Bir ATOMIC chunk'ını işler: STRUCT(16) + GEOMETRY peek/decode.
 *
 * @param decode TRUE → Geometry::streamRead + Atomic oluştur (clump'a ekle);
 *               FALSE → yalnız envanter say (Inspect).
 * @return decode modunda: oluşturulan atomic (başarısız geometride NULL);
 *         peek modunda: her zaman dummy (1).
 */
RwBool ProcessAtomic(Cursor &c, RwBool decode, rw::Clump *pClump,
                     rw::Frame *pRootFrame, RwUInt32 *pVerts,
                     RwUInt32 *pTris, rw::Atomic **ppAtomic) {
    if (ppAtomic) {
        *ppAtomic = nullptr;
    }
    RwUInt32 atomicLen = 0;
    if (!ReadHeader(c, kChunkAtomic, &atomicLen)) {
        return FALSE;
    }
    const RwUInt32 atomicEnd = c.pos + atomicLen;

    // inner STRUCT(16)
    RwUInt32 structLen = 0;
    if (!ReadHeader(c, kChunkStruct, &structLen) || structLen != 16) {
        c.pos = atomicEnd;
        return FALSE;
    }
    c.pos += 16;

    // inner GEOMETRY — STRUCT başlığından flags/tris/verts peek
    const RwUInt32 geoChunkStart = c.pos;
    RwUInt32 geoLen = 0;
    if (!ReadHeader(c, kChunkGeometry, &geoLen)) {
        c.pos = atomicEnd;
        return FALSE;
    }
    // c.pos = geometry gövdesi = STRUCT header (Geometry::streamRead bunu
    // findChunk ile okur). Peek: [type][len][libid][GeoStreamData].
    if (!Have(c, 12 + 16)) {
        c.pos = atomicEnd;
        return FALSE;
    }
    const RwUInt32 stType = ReadU32(c);
    if (stType != kChunkStruct) {
        c.pos = atomicEnd;
        return FALSE;
    }
    c.pos += 4; // STRUCT type'ı geç (c.pos = len alanının başı)
    const RwUInt32 flags = ReadU32At(c.pBuf, c.pos + 8);
    const RwUInt32 numTris = ReadU32At(c.pBuf, c.pos + 12);
    const RwUInt32 numVerts = ReadU32At(c.pBuf, c.pos + 16);
    *pVerts += numVerts;
    *pTris += numTris;
    (void)flags;

    if (decode && pClump != nullptr) {
        // stream'i geometry gövdesinin BAŞINA konumla (findChunk STRUCT
        // bekler — geometry header'dan hemen sonra).
        rw::StreamMemory mem;
        mem.open(const_cast<RwUInt8 *>(c.pBuf) + geoChunkStart + 12,
                 geoLen, geoLen);
        rw::Geometry *pGeo = rw::Geometry::streamRead(&mem);
        mem.close();
        if (pGeo != nullptr) {
            rw::Atomic *pAtomic = rw::Atomic::create();
            rw::Frame *pFrame = rw::Frame::create();
            if (pAtomic != nullptr && pFrame != nullptr) {
                pAtomic->setFrame(pFrame);
                pRootFrame->addChild(pFrame); // köke bağla (hiyerarşi)
                pAtomic->setGeometry(pGeo, 0);
                pGeo->destroy(); // setGeometry ref alır
                pClump->addAtomic(pAtomic);
                if (ppAtomic) {
                    *ppAtomic = pAtomic;
                }
            } else {
                if (pAtomic) {
                    pAtomic->destroy();
                }
                if (pFrame) {
                    pFrame->destroy();
                }
                pGeo->destroy();
            }
        }
    }

    c.pos = atomicEnd;
    return TRUE;
}

} // namespace

RwBool WorldGeometry_Inspect(const void *pBuf, RwUInt32 len,
                             WorldGeometryInfo *pInfo) {
    if (pInfo) {
        std::memset(pInfo, 0, sizeof(*pInfo));
    }
    if (pBuf == nullptr || len < 8 + 12 + 12) {
        return FALSE;
    }
    Cursor c = {static_cast<const RwUInt8 *>(pBuf), len, 0};
    const RwUInt32 preLen = ReadU32(c);
    c.pos += 4;
    const RwUInt32 libid = ReadU32(c);
    c.pos += 4;
    if (libid != kLibraryIdRW36) {
        return FALSE;
    }
    if (preLen < 12 || 8 + preLen > len) {
        return FALSE;
    }

    // Prefix: STRUCT(16) + GEOMETRY + EXTENSION — preLen byte
    {
        Cursor prefix = c;
        const RwUInt32 prefixEnd = 8 + preLen;
        RwUInt32 structLen = 0;
        if (!ReadHeader(prefix, kChunkStruct, &structLen) || structLen != 16 ||
            prefixEnd > len) {
            return FALSE;
        }
        if (pInfo) {
            std::memcpy(pInfo->masterStruct, prefix.pBuf + prefix.pos, 16);
        }
        prefix.pos += 16;
        // GEOMETRY tam atlanır (başlık + geoLen)
        RwUInt32 geoLen = 0;
        if (!ReadHeader(prefix, kChunkGeometry, &geoLen)) {
            return FALSE;
        }
        prefix.pos += geoLen;
        // EXTENSION (uzunluk 0 olabilir)
        RwUInt32 extLen = 0;
        if (!ReadHeader(prefix, 0x03, &extLen)) {
            return FALSE;
        }
        prefix.pos += extLen;
        if (prefix.pos != prefixEnd) {
            return FALSE; // preLen tutarlılığı
        }
    }

    // ATOMIC döngüsü — binary FUN_004c9b70 gibi
    c.pos = 8 + preLen;
    RwUInt32 atomicCount = 0;
    RwUInt32 totalVerts = 0;
    RwUInt32 totalTris = 0;
    while (c.pos + 12 <= len) {
        const RwUInt32 type = ReadU32(c);
        if (type != kChunkAtomic) {
            break; // trailing chunk
        }
        if (!ProcessAtomic(c, FALSE, nullptr, nullptr, &totalVerts,
                           &totalTris, nullptr)) {
            return FALSE; // bozuk atomic — yapısal hata
        }
        ++atomicCount;
    }
    if (atomicCount == 0) {
        return FALSE;
    }

    if (pInfo) {
        pInfo->prefixSectionLength = preLen;
        pInfo->libraryId = libid;
        pInfo->atomicCount = atomicCount;
        pInfo->totalVertices = totalVerts;
        pInfo->totalTriangles = totalTris;
        pInfo->trailingChunkType = (c.pos + 12 <= len) ? ReadU32(c) : 0;
    }
    return TRUE;
}

void *WorldGeometry_CreateClump(const void *pBuf, RwUInt32 len) {
    WorldGeometryInfo info;
    if (WorldGeometry_Inspect(pBuf, len, &info) != TRUE) {
        return nullptr;
    }

    // auto-framing bbox — her CreateClump sıfırlanır (son dünya geçerli)
    s_Bounds.valid = FALSE;

    rw::Clump *pClump = rw::Clump::create();
    rw::Frame *pRootFrame = rw::Frame::create();
    if (pClump == nullptr || pRootFrame == nullptr) {
        if (pClump) {
            pClump->destroy();
        }
        if (pRootFrame) {
            pRootFrame->destroy();
        }
        return nullptr;
    }
    pClump->setFrame(pRootFrame);

    Cursor c = {static_cast<const RwUInt8 *>(pBuf), len,
                8 + info.prefixSectionLength};
    RwUInt32 verts = 0;
    RwUInt32 tris = 0;
    RwUInt32 loaded = 0;
    while (c.pos + 12 <= len) {
        if (ReadU32(c) != kChunkAtomic) {
            break; // ReadU32 position ilerletmez
        }
        rw::Atomic *pAtomic = nullptr;
        if (ProcessAtomic(c, TRUE, pClump, pRootFrame, &verts, &tris,
                          &pAtomic) != TRUE) {
            break;
        }
        if (pAtomic != nullptr) {
            ++loaded;
            // bbox birikimi — tüm atomic vertice'leri (auto-framing)
            rw::Geometry *pGeo = pAtomic->geometry;
            if (pGeo != nullptr && pGeo->numMorphTargets > 0 &&
                pGeo->morphTargets[0].vertices != nullptr) {
                const rw::V3d *pv = pGeo->morphTargets[0].vertices;
                for (int i = 0; i < pGeo->numVertices; ++i) {
                    if (s_Bounds.valid != TRUE) {
                        s_Bounds.min[0] = s_Bounds.max[0] = pv[i].x;
                        s_Bounds.min[1] = s_Bounds.max[1] = pv[i].y;
                        s_Bounds.min[2] = s_Bounds.max[2] = pv[i].z;
                        s_Bounds.valid = TRUE;
                    } else {
                        if (pv[i].x < s_Bounds.min[0]) {
                            s_Bounds.min[0] = pv[i].x;
                        }
                        if (pv[i].x > s_Bounds.max[0]) {
                            s_Bounds.max[0] = pv[i].x;
                        }
                        if (pv[i].y < s_Bounds.min[1]) {
                            s_Bounds.min[1] = pv[i].y;
                        }
                        if (pv[i].y > s_Bounds.max[1]) {
                            s_Bounds.max[1] = pv[i].y;
                        }
                        if (pv[i].z < s_Bounds.min[2]) {
                            s_Bounds.min[2] = pv[i].z;
                        }
                        if (pv[i].z > s_Bounds.max[2]) {
                            s_Bounds.max[2] = pv[i].z;
                        }
                    }
                }
            }
        }
    }

    if (loaded == 0) {
        // Clump::destroy frame hiyerarşisini de yok eder (getFrame →
        // destroyHierarchy) — ayrıca destroyHierarchy ÇAĞRILMAZ.
        pClump->destroy();
        return nullptr;
    }
    std::fprintf(stderr,
                 "WorldGeometry: %u/%u atomics instantiated (%u verts)\n",
                 loaded, info.atomicCount, verts);
    return pClump;
}

void WorldGeometry_DestroyClump(void *pClumpPtr) {
    if (pClumpPtr == nullptr) {
        return;
    }
    rw::Clump *pClump = static_cast<rw::Clump *>(pClumpPtr);
    // Clump::destroy atomics + geometriler + frame hiyerarşisini (setFrame
    // edilen kök frame dahil) yok eder.
    pClump->destroy();
}

RwBool WorldGeometry_GetLastBounds(WorldGeometryBounds *pOut) {
    if (pOut == nullptr) {
        return FALSE;
    }
    *pOut = s_Bounds;
    return s_Bounds.valid;
}

void WorldGeometry_Diagnose(const void *pClumpPtr, int maxAtomics) {
    if (pClumpPtr == nullptr || maxAtomics <= 0) {
        return;
    }
    // Teşhis boyası: CANNONCRUISE_REDGEO=1 → tüm vertex renkleri kırmızı
    // (ilk render öncesi instance'a girer; renk yolu testi).
    const RwBool paintRed = (std::getenv("CANNONCRUISE_REDGEO") != nullptr)
                                ? TRUE
                                : FALSE;
    rw::Clump *pClump = static_cast<rw::Clump *>(
        const_cast<void *>(pClumpPtr));
    if (paintRed == TRUE) {
        // Atomik başına düz renk (kırmızı/yeşil/mavi/sarı/magenta/cyan).
        static const RwUInt8 kCols[6][3] = {
            {255, 16, 16}, {16, 255, 16}, {16, 16, 255},
            {255, 255, 16}, {255, 16, 255}, {16, 255, 255}};
        int ai = 0;
        FORLIST(lnk, pClump->atomics) {
            rw::Atomic *pAtomic = LLLinkGetData(lnk, rw::Atomic, inClump);
            rw::Geometry *g =
                (pAtomic != nullptr) ? pAtomic->geometry : nullptr;
            if (g != nullptr && g->colors != nullptr) {
                const RwUInt8 *cc = kCols[ai % 6];
                for (int v = 0; v < g->numVertices; ++v) {
                    g->colors[v].red = cc[0];
                    g->colors[v].green = cc[1];
                    g->colors[v].blue = cc[2];
                    g->colors[v].alpha = 255;
                }
            }
            ++ai;
        }
    }
    // İzolasyon: CANNONCRUISE_ONEATOMIC=1 → ilk atomic hariç hepsini sök
    // (üzerine çizim/overdraw şüphesi).
    if (std::getenv("CANNONCRUISE_ONEATOMIC") != nullptr) {
        bool first = true;
        FORLIST(lnk, pClump->atomics) {
            rw::Atomic *pAtomic = LLLinkGetData(lnk, rw::Atomic, inClump);
            if (first) {
                first = false;
                continue;
            }
            if (pAtomic != nullptr) {
                pClump->removeAtomic(pAtomic);
            }
        }
    }
    // Culling kapatma teşhisi: CANNONCRUISE_NOCULL=1 → tüm küreler devasa.
    if (std::getenv("CANNONCRUISE_NOCULL") != nullptr) {
        FORLIST(lnk, pClump->atomics) {
            rw::Atomic *pAtomic = LLLinkGetData(lnk, rw::Atomic, inClump);
            if (pAtomic != nullptr) {
                pAtomic->boundingSphere.radius = 1e9f;
                pAtomic->object.object.privateFlags |=
                    rw::Atomic::WORLDBOUNDDIRTY;
            }
        }
    }
    int n = 0;
    FORLIST(lnk, pClump->atomics) {
        if (n >= maxAtomics) {
            break;
        }
        rw::Atomic *pAtomic = LLLinkGetData(lnk, rw::Atomic, inClump);
        rw::Geometry *g =
            (pAtomic != nullptr) ? pAtomic->geometry : nullptr;
        if (g == nullptr) {
            std::printf("GeoDiag [%d]: null geometry\n", n);
            ++n;
            continue;
        }
        const RwBool prelit = (g->flags & rw::Geometry::PRELIT) ? TRUE : FALSE;
        const RwBool textured =
            (g->flags & rw::Geometry::TEXTURED) ? TRUE : FALSE;
        char vcol[32] = "none";
        if (g->colors != nullptr && g->numVertices > 0) {
            std::snprintf(vcol, sizeof(vcol), "(%u,%u,%u,%u)",
                          (unsigned)g->colors[0].red,
                          (unsigned)g->colors[0].green,
                          (unsigned)g->colors[0].blue,
                          (unsigned)g->colors[0].alpha);
            // Renk histogramı (tüm verteksler): [0] temsilci mi?
            long sumR = 0, sumG = 0, sumB = 0;
            int nBlack = 0;
            for (int v = 0; v < g->numVertices; ++v) {
                sumR += g->colors[v].red;
                sumG += g->colors[v].green;
                sumB += g->colors[v].blue;
                if (g->colors[v].red < 16 && g->colors[v].green < 16 &&
                    g->colors[v].blue < 16) {
                    ++nBlack;
                }
            }
            const int nv = (g->numVertices > 0) ? g->numVertices : 1;
            int nA0 = 0, nAFF = 0, nAOther = 0;
            for (int v = 0; v < g->numVertices; ++v) {
                if (g->colors[v].alpha == 0) {
                    ++nA0;
                } else if (g->colors[v].alpha == 255) {
                    ++nAFF;
                } else {
                    ++nAOther;
                }
            }
            std::printf("GeoDiag [%d] coloravg=(%ld,%ld,%ld) black=%d/%d "
                        "alpha0=%d alphaFF=%d alphaMid=%d\n",
                        n, sumR / nv, sumG / nv, sumB / nv, nBlack, nv, nA0,
                        nAFF, nAOther);
        }
        char nrm[32] = "none";
        if (g->morphTargets != nullptr && g->numMorphTargets > 0 &&
            g->morphTargets[0].normals != nullptr && g->numVertices > 0) {
            std::snprintf(nrm, sizeof(nrm), "(%.2f,%.2f,%.2f)",
                          (double)g->morphTargets[0].normals[0].x,
                          (double)g->morphTargets[0].normals[0].y,
                          (double)g->morphTargets[0].normals[0].z);
        }
        std::printf("GeoDiag [%d]: verts=%d flags=0x%x prelit=%d textured=%d "
                    "vcol0=%s nrm0=%s mats=%d uvsets=%d",
                    n, (int)g->numVertices, (unsigned)g->flags, (int)prelit,
                    (int)textured, vcol, nrm, (int)g->matList.numMaterials,
                    (int)g->numTexCoordSets);
        if (g->numTexCoordSets > 0 && g->texCoords[0] != nullptr &&
            g->numVertices > 0) {
            std::printf(" uv0=(%.3f,%.3f)", (double)g->texCoords[0][0].u,
                        (double)g->texCoords[0][0].v);
        }
        if (pAtomic->getFrame() != nullptr) {
            const rw::Sphere *sp = nullptr;
            if (g->morphTargets != nullptr && g->numMorphTargets > 0) {
                sp = &g->morphTargets[0].boundingSphere;
            }
            if (sp != nullptr) {
                std::printf(" bsphere=(%.1f,%.1f,%.1f,r=%.1f)",
                            (double)sp->center.x, (double)sp->center.y,
                            (double)sp->center.z, (double)sp->radius);
            }
        }
        std::printf("\n");
        for (int m = 0; m < g->matList.numMaterials && m < 4; ++m) {
            const rw::Material *mat = g->matList.materials[m];
            if (mat == nullptr) {
                std::printf("GeoDiag [%d] mat%d: null\n", n, m);
                continue;
            }
            const rw::Texture *tex = mat->texture;
            char tinfo[96] = "none";
            if (tex != nullptr) {
                std::snprintf(tinfo, sizeof(tinfo), "'%s' raster=%s", tex->name,
                              (tex->raster != nullptr) ? "ok" : "NULL");
            }
            std::printf("GeoDiag [%d] mat%d: color=(%u,%u,%u,%u) tex=%s "
                        "pipe=%p\n",
                        n, m, (unsigned)mat->color.red,
                        (unsigned)mat->color.green,
                        (unsigned)mat->color.blue,
                        (unsigned)mat->color.alpha, tinfo,
                        mat->pipeline);
        }
        ++n;
    }
}

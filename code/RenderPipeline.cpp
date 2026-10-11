// ============================================================================
// CannonCruise - Render Pipeline state (RenderPipeline.cpp)
// Original path: D:\Projects\CannonCruisePC\code\RenderPipeline.cpp
// Tersine mühendislik ile CC_ida.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları (gövde karşılıkları — başlık bloğuna bkz.):
//   FUN_004cc1f0 / FUN_004cbd20 / FUN_004cc2f0 / FUN_004cc350 /
//   FUN_004cc480 / FUN_004cc4d0 / FUN_004cc020 (çekirdek)
//
// Matris yığını slot[32] sabit dizide modellenir (binary'de işaretçi
// havuzu + lazy sub_510600 tahsisi; port statik — tek iş parçacığı).
// ============================================================================

#include "RenderPipeline.h"

#include <cstring>

namespace
{
    const RwInt32 kStackSlots = 32; // binary: cmp eax,1Fh (0..31)

    // dword_66D644[0..31] — yığın slot matrisleri; dword_66D6C4 derinlik
    CCMatrix s_MatrixStack[kStackSlots];
    RwInt32  s_StackDepth = 0; // dword_66D6C4

    // dword_66D604 — kök/global matris (sub_4cc1f0 kimlik; sub_4cc520
    // sub_50f760 ile üst matrisle çarpar — portlanmadı, kayıt için)
    CCMatrix s_RootMatrix;

    // dword_66D7C0 / dword_66D7C4 — en-boy çifti (sub_4cc020)
    RwReal s_AspectX = 1.0f;
    RwReal s_AspectY = 1.0f;

    // dword_66D6D0/D4 — raster genişlik/yükseklik; flt_66D6C8/CC — görüş
    // penceresi ×2 (sub_4cc020)
    RwReal s_RasterWidth = 0.0f;
    RwReal s_RasterHeight = 0.0f;
    RwReal s_ViewWinX2 = 0.0f;
    RwReal s_ViewWinY2 = 0.0f;

    // dword_66D7A0 z / dword_66D7A4 half-pixel (sub_4cc1f0 başlatır)
    RwReal s_DepthZ = 1.0f;
    RwReal s_HalfPixel = 0.5f;

    // dword_66D798 / dword_66D7AC — önbellek geçerlilik bayrakları
    // (sub_4cc480/4cc4d0/4cbd20 sıfırlar)
    RwInt32 s_ParamsValid = 0;
    RwInt32 s_CacheFlag2 = 0;

    // kamera bağlamı (sub_4cc020/4cc120/4cc130/4cc520)
    const RCCCameraView *s_pCameraView = 0; // dword_66D7CC
    RwReal s_OffX = 0.0f;                   // dword_66D7B8
    RwReal s_OffY = 0.0f;                   // dword_66D7BC
    void *s_pContext = 0;                   // dword_66D79C
    RwInt32 s_FlagsB0 = 7;                  // dword_66D7B0 (Reset değeri)

    // sub_4cc520 param bloğu — unk_66D6D8 (ölçek), unk_66D718 (ters),
    // unk_66D758 (ölçek × frame — her çağrıda)
    CCMatrix s_ViewScale;
    CCMatrix s_ViewInverse;
    CCMatrix s_ViewResult;

    // 0x20003 — birimlik işaret maskesi (sub_4cc1f0/sub_4cbd20: flags |=)
    const RwInt32 kMatrixFlags = 0x20003;

    void MatrixIdentityInto(CCMatrix *pM) {
        std::memset(pM, 0, sizeof(*pM));
        pM->right[0] = 1.0f;
        pM->up[1] = 1.0f;
        pM->at[2] = 1.0f;
        pM->flags |= kMatrixFlags;
    }

    // sub_50f760 (RwMatrixMultiply) — out = a × b: b önce uygulanır
    // (v' = v·b·a); 3x3 sütun çarpımı + pos = a.3x3×b.pos + a.pos.
    void MatrixMultiplyInto(CCMatrix *pOut, const CCMatrix *pA,
                            const CCMatrix *pB) {
        CCMatrix r;
        const RwReal *aCols[3] = {pA->right, pA->up, pA->at};
        const RwReal *bCols[3] = {pB->right, pB->up, pB->at};
        RwReal *oCols[3] = {r.right, r.up, r.at};
        for (int col = 0; col < 3; ++col) {
            for (int row = 0; row < 3; ++row) {
                oCols[col][row] = aCols[0][row] * bCols[col][0] +
                                  aCols[1][row] * bCols[col][1] +
                                  aCols[2][row] * bCols[col][2];
            }
        }
        for (int row = 0; row < 3; ++row) {
            r.pos[row] = pA->right[row] * pB->pos[0] +
                         pA->up[row] * pB->pos[1] +
                         pA->at[row] * pB->pos[2] + pA->pos[row];
        }
        r.flags = pA->flags | pB->flags;
        r.pad0 = 0;
        r.pad1 = 0;
        r.pad2 = 0;
        *pOut = r;
    }

    // sub_510170 kombinasyon tip 2 (sub_4cc520 kullanır) — tam ölçek:
    // sütunlar × (v.x, v.y, v.z) + pos × (v.x, v.y, v.z); flags
    // &= 0xFFFDFFFC (birimlik/ölçek bayrakları temiz). Tip 1 (sub_4cc480)
    // yalnız sütun ölçekler — mevcut MatrixScaleXY doğru.
    void MatrixScaleXYZInto(CCMatrix *pM, RwReal x, RwReal y, RwReal z) {
        pM->right[0] *= x;
        pM->right[1] *= x;
        pM->right[2] *= x;
        pM->up[0] *= y;
        pM->up[1] *= y;
        pM->up[2] *= y;
        pM->at[0] *= z;
        pM->at[1] *= z;
        pM->at[2] *= z;
        pM->pos[0] *= x;
        pM->pos[1] *= y;
        pM->pos[2] *= z;
        pM->flags &= static_cast<RwInt32>(0xFFFDFFFCu);
    }

    // sub_510300 kombinasyon tip 2 (sub_4cc520 kullanır) — ham öteleme:
    // pos += v (sütunlara dokunmaz); flags &= 0xFFFDFFFF.
    void MatrixTranslateRawInto(CCMatrix *pM, RwReal x, RwReal y, RwReal z) {
        pM->pos[0] += x;
        pM->pos[1] += y;
        pM->pos[2] += z;
        pM->flags &= static_cast<RwInt32>(0xFFFDFFFFu);
    }

    // sub_50ff20 (RwMatrixInvert) — kimlik kopyala (flags&0x20000),
    // orthonormal hızlı (flags&3==3: transpoze + neg-poz), genel yol
    // sub_510010 (adjugate/det; det==0 → katsayı 1.0 — flt_5EF2E0).
    void MatrixInvertInto(CCMatrix *pOut, const CCMatrix *pM) {
        if (pM->flags & 0x20000) {
            *pOut = *pM; // kimlik önbellek kopyası (rep movsd 10h)
            return;
        }
        if ((pM->flags & 3) == 3) {
            // orthonormal: out.sütun = in.sütun satırları (transpoze);
            // out.pos = -(pos·right, pos·up, pos·at); flags = 3
            CCMatrix r;
            for (int row = 0; row < 3; ++row) {
                r.right[row] = pM->right[row];
                r.up[row] = pM->up[row];
                r.at[row] = pM->at[row];
            }
            for (int row = 0; row < 3; ++row) {
                r.pos[row] = -(pM->pos[0] * pM->right[row] +
                               pM->pos[1] * pM->up[row] +
                               pM->pos[2] * pM->at[row]);
            }
            r.flags = 3;
            r.pad0 = 0;
            r.pad1 = 0;
            r.pad2 = 0;
            *pOut = r;
            return;
        }
        // genel afin ters — 3x3 adjugate/det + pos = -out.3x3 × in.pos
        const RwReal *b[3] = {pM->right, pM->up, pM->at};
        const RwReal det =
            b[0][0] * (b[2][1] * b[1][2] - b[1][1] * b[2][2]) -
            b[1][0] * (b[2][1] * b[0][2] - b[0][1] * b[2][2]) +
            b[2][0] * (b[1][1] * b[0][2] - b[0][1] * b[1][2]);
        const RwReal invScale = (det != 0.0f) ? (1.0f / det) : 1.0f;
        CCMatrix r;
        r.right[0] = (b[2][1] * b[1][2] - b[1][1] * b[2][2]) * invScale;
        r.right[1] = -(b[2][1] * b[0][2] - b[0][1] * b[2][2]) * invScale;
        r.right[2] = (b[1][1] * b[0][2] - b[0][1] * b[1][2]) * invScale;
        r.up[0] = -(b[2][0] * b[1][2] - b[1][0] * b[2][2]) * invScale;
        r.up[1] = (b[2][0] * b[0][2] - b[0][0] * b[2][2]) * invScale;
        r.up[2] = -(b[1][0] * b[0][2] - b[0][0] * b[1][2]) * invScale;
        r.at[0] = (b[2][0] * b[1][1] - b[1][0] * b[2][1]) * invScale;
        r.at[1] = -(b[2][0] * b[0][1] - b[0][0] * b[2][1]) * invScale;
        r.at[2] = (b[1][0] * b[0][1] - b[0][0] * b[1][1]) * invScale;
        for (int row = 0; row < 3; ++row) {
            r.pos[row] = -(r.right[row] * pM->pos[0] +
                           r.up[row] * pM->pos[1] +
                           r.at[row] * pM->pos[2]);
        }
        r.flags = 0;
        r.pad0 = 0;
        r.pad1 = 0;
        r.pad2 = 0;
        *pOut = r;
    }
}

// ---------------------------------------------------------------------------
// RenderPipeline_Reset — sub_4CC1F0
// ---------------------------------------------------------------------------
RwBool RenderPipeline_Reset(void) {
    s_StackDepth = 0;                    // dword_66D6C4 = 0
    MatrixIdentityInto(&s_MatrixStack[0]); // dword_66D644[0] = birim
    MatrixIdentityInto(&s_RootMatrix);   // dword_66D604 kimlik
    s_RootMatrix.flags |= kMatrixFlags;  // dword_66D610 |= 0x20003
    s_DepthZ = 1.0f;                     // dword_66D7A0
    s_HalfPixel = 0.5f;                  // dword_66D7A4
    s_ParamsValid = 0;                   // dword_66D798
    s_CacheFlag2 = 0;                    // dword_66D7AC
    s_AspectX = 1.0f;                    // flt_66D7C0
    s_AspectY = 1.0f;                    // dword_66D7C4
    s_pCameraView = 0;                   // dword_66D7CC
    s_OffX = 0.0f;                       // dword_66D7B8
    s_OffY = 0.0f;                       // dword_66D7BC
    s_FlagsB0 = 7;                       // dword_66D7B0
    MatrixIdentityInto(&s_ViewScale);    // unk_66D6D8 (init)
    MatrixIdentityInto(&s_ViewInverse);  // unk_66D718 (init)
    MatrixIdentityInto(&s_ViewResult);   // unk_66D758 (init)
    for (RwInt32 i = 1; i < kStackSlots; ++i) {
        MatrixIdentityInto(&s_MatrixStack[i]);
    }
    return TRUE;
}

// ---------------------------------------------------------------------------
// RenderPipeline_MatrixIdentity — sub_4CBD20
// ---------------------------------------------------------------------------
RwBool RenderPipeline_MatrixIdentity(void) {
    CCMatrix *pTop = &s_MatrixStack[s_StackDepth];
    MatrixIdentityInto(pTop);
    s_ParamsValid = 0; // dword_66D798 = 0
    s_CacheFlag2 = 0;  // dword_66D7AC = 0
    return TRUE;
}

// ---------------------------------------------------------------------------
// RenderPipeline_MatrixPush — sub_4CC2F0
// ---------------------------------------------------------------------------
RwBool RenderPipeline_MatrixPush(void) {
    if (s_StackDepth >= kStackSlots - 1) {
        return FALSE; // binary: cmp 1Fh / setl — taşma
    }
    ++s_StackDepth;
    s_MatrixStack[s_StackDepth] = s_MatrixStack[s_StackDepth - 1];
    return TRUE;
}

// ---------------------------------------------------------------------------
// RenderPipeline_MatrixPushCopy — sub_4CC350
// ---------------------------------------------------------------------------
RwBool RenderPipeline_MatrixPushCopy(const CCMatrix *pSrc) {
    if (s_StackDepth >= kStackSlots - 1) {
        return FALSE;
    }
    ++s_StackDepth;
    if (!pSrc) {
        // ebeveyn kopyası (sub_4cc350 arg_0==0 dalı)
        s_MatrixStack[s_StackDepth] = s_MatrixStack[s_StackDepth - 1];
    } else {
        // sub_50f760(dest=üst, arg_0, ebeveyn) — ebeveyn önce uygulanır
        MatrixMultiplyInto(&s_MatrixStack[s_StackDepth], pSrc,
                           &s_MatrixStack[s_StackDepth - 1]);
    }
    s_ParamsValid = 0; // dword_66D798 = 0
    return TRUE;
}

// ---------------------------------------------------------------------------
// RenderPipeline_MatrixScaleXY — sub_4CC480 (sub_510170 → RwMatrixScale)
// ---------------------------------------------------------------------------
RwBool RenderPipeline_MatrixScaleXY(RwReal x, RwReal y) {
    CCMatrix *pTop = &s_MatrixStack[s_StackDepth];
    pTop->right[0] *= x;
    pTop->right[1] *= x;
    pTop->right[2] *= x;
    pTop->up[0] *= y;
    pTop->up[1] *= y;
    pTop->up[2] *= y;
    // at sütunu ×1 (binary var_4 = 3F800000h)
    s_ParamsValid = 0; // dword_66D798 = 0
    s_CacheFlag2 = 0;  // dword_66D7AC = 0
    return TRUE;
}

// ---------------------------------------------------------------------------
// RenderPipeline_MatrixTranslateXY — sub_4CC4D0 (sub_510300 → RwMatrixTranslate)
// pos += right×x + up×y (var_4 = 0 — Z bileşeni yok)
// ---------------------------------------------------------------------------
RwBool RenderPipeline_MatrixTranslateXY(RwReal x, RwReal y) {
    CCMatrix *pTop = &s_MatrixStack[s_StackDepth];
    pTop->pos[0] += pTop->right[0] * x + pTop->up[0] * y;
    pTop->pos[1] += pTop->right[1] * x + pTop->up[1] * y;
    pTop->pos[2] += pTop->right[2] * x + pTop->up[2] * y;
    s_ParamsValid = 0; // dword_66D798 = 0
    s_CacheFlag2 = 0;  // dword_66D7AC = 0
    return TRUE;
}

// ---------------------------------------------------------------------------
// RenderPipeline_SetViewWindow — sub_4CC020 taşınabilir çekirdeği
// ---------------------------------------------------------------------------
RwBool RenderPipeline_SetViewWindow(RwReal width, RwReal height,
                                    RwReal winX2, RwReal winY2) {
    s_RasterWidth = width;   // flt_66D6D0
    s_RasterHeight = height; // flt_66D6D4
    s_ViewWinX2 = winX2;     // flt_66D6C8 = [kamera+0x68]×2
    s_ViewWinY2 = winY2;     // flt_66D6CC = [kamera+0x6C]×2
    if (winX2 >= winY2) {
        s_AspectY = 1.0f;                    // dword_66D7C4 = 1.0
        s_AspectX = (winY2 != 0.0f) ? winX2 / winY2 : 1.0f;
    } else {
        s_AspectX = 1.0f;                    // flt_66D7C0 = 1.0
        s_AspectY = (winX2 != 0.0f) ? winY2 / winX2 : 1.0f;
    }
    s_ParamsValid = 0; // dword_66D798 = 0
    return TRUE;
}

// ---------------------------------------------------------------------------
// test erişimi
// ---------------------------------------------------------------------------
const CCMatrix *RenderPipeline_GetTopMatrix(void) {
    return &s_MatrixStack[s_StackDepth];
}

RwInt32 RenderPipeline_GetStackDepth(void) {
    return s_StackDepth;
}

void RenderPipeline_GetAspect(RwReal *pAspectX, RwReal *pAspectY) {
    if (pAspectX) {
        *pAspectX = s_AspectX;
    }
    if (pAspectY) {
        *pAspectY = s_AspectY;
    }
}

void RenderPipeline_GetViewWindow(RwReal *pWidth, RwReal *pHeight,
                                  RwReal *pWinX2, RwReal *pWinY2) {
    if (pWidth) {
        *pWidth = s_RasterWidth;
    }
    if (pHeight) {
        *pHeight = s_RasterHeight;
    }
    if (pWinX2) {
        *pWinX2 = s_ViewWinX2;
    }
    if (pWinY2) {
        *pWinY2 = s_ViewWinY2;
    }
}

const CCMatrix *RenderPipeline_GetViewScaleMatrix(void) {
    return &s_ViewScale;
}

const CCMatrix *RenderPipeline_GetViewInverseMatrix(void) {
    return &s_ViewInverse;
}

// ---------------------------------------------------------------------------
// RenderPipeline_SetViewWindowFromCamera — sub_4CC020 tam yolu
// ---------------------------------------------------------------------------
RwBool RenderPipeline_SetViewWindowFromCamera(const RCCCameraView *pCam) {
    if (!pCam) {
        s_pCameraView = 0; // dword_66D7CC = NULL (loc_4CC103)
        return FALSE;
    }
    // raster == NULL → FALSE; dword_66D7CC DEĞİŞMEZ (loc_4CC109 — binary
    // kamera önbelleğini korur)
    if (pCam->rasterWidth <= 0 || pCam->rasterHeight <= 0) {
        return FALSE;
    }
    // vtable kancası atlantır (motor tarafı — başlık bloğu)
    RenderPipeline_SetViewWindow(static_cast<RwReal>(pCam->rasterWidth),
                                 static_cast<RwReal>(pCam->rasterHeight),
                                 pCam->viewWindowX * 2.0f,
                                 pCam->viewWindowY * 2.0f);
    s_pCameraView = pCam; // dword_66D7CC = kamera (loc_4CC08A)
    s_OffX = 0.0f;        // dword_66D7B8 = 0
    s_OffY = 0.0f;        // dword_66D7BC = 0
    // dword_66D798 = 0 — SetViewWindow çekirdeği zaten sıfırlar
    return TRUE;
}

const RCCCameraView *RenderPipeline_GetCamera(void) {
    return s_pCameraView; // sub_4cc120 — dword_66D7CC
}

// ---------------------------------------------------------------------------
// RenderPipeline_SetContext / ClearContext / GetContext — sub_4CC130
// (dword_66D79C — oyunun geçerli çizim bağlamı)
// ---------------------------------------------------------------------------
RwBool RenderPipeline_SetContext(void *pContext) {
    s_pContext = pContext; // sub_4cc130: dword_66D79C = arg_0
    return TRUE;
}

void RenderPipeline_ClearContext(void) {
    s_pContext = 0; // sub_4cc140 ilk adım: dword_66D79C = 0
}

void *RenderPipeline_GetContext(void) {
    return s_pContext;
}

// ---------------------------------------------------------------------------
// RenderPipeline_GetViewParams — sub_4CBE10
// ---------------------------------------------------------------------------
void RenderPipeline_GetViewParams(RwReal *pOffX, RwReal *pOffY,
                                  RwReal *pAspectX, RwReal *pAspectY) {
    if (pOffX) {
        *pOffX = s_OffX; // dword_66D7B8
    }
    if (pOffY) {
        *pOffY = s_OffY; // dword_66D7BC
    }
    RenderPipeline_GetAspect(pAspectX, pAspectY);
}

// ---------------------------------------------------------------------------
// RenderPipeline_BuildViewMatrix — sub_4CC520
// ---------------------------------------------------------------------------
const CCMatrix *RenderPipeline_BuildViewMatrix(void) {
    if (!s_ParamsValid) {
        // zScale: kamera projection tipi 2 → 1.0, değilse dword_66D7A0
        const RwReal zScale = (s_pCameraView && s_pCameraView->projFlag == 2)
                                  ? 1.0f
                                  : s_DepthZ;
        // m = üst × kök (sub_50f760 — kök önce uygulanır)
        CCMatrix m;
        MatrixMultiplyInto(&m, &s_MatrixStack[s_StackDepth], &s_RootMatrix);
        // ölçek(zScale, zScale, 1) — sub_510170 tip 2
        MatrixScaleXYZInto(&m, zScale, zScale, 1.0f);
        // w = min(winX2, winY2); ölçek(-w, w, 1) — sub_510170 tip 2
        const RwReal w = (s_ViewWinX2 >= s_ViewWinY2) ? s_ViewWinY2
                                                       : s_ViewWinX2;
        MatrixScaleXYZInto(&m, -w, w, 1.0f);
        // translate(winX2×zScale×0.5, -winY2×zScale×0.5, 0) — sub_510300
        // tip 2 (flt_5EF624 = 0.5)
        MatrixTranslateRawInto(&m, s_ViewWinX2 * zScale * 0.5f,
                               -s_ViewWinY2 * zScale * 0.5f, 0.0f);
        s_ViewScale = m;                          // unk_66D6D8 (cache)
        MatrixInvertInto(&s_ViewInverse, &m);     // unk_66D718 (sub_50ff20)
        s_ParamsValid = 1;                        // dword_66D798 = 1
    }
    // sonuç = ölçek × kamera frame — HER ÇAĞRIDA (kamera hareketi
    // önbelleği tazelemez; sub_50e990 frame LTM)
    if (s_pCameraView && s_pCameraView->pFrameMatrix) {
        MatrixMultiplyInto(&s_ViewResult, &s_ViewScale,
                           s_pCameraView->pFrameMatrix);
        return &s_ViewResult; // unk_66D758
    }
    // port kılavuzu: kamera/kare yoksa ölçek matrisi döner (binary
    // dword_66D7CC'e kilitlenirdi — [eax+4] doğrudan)
    return &s_ViewScale;
}

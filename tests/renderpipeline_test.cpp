// SDK-independent regression test for the portable render pipeline state
// (matrix stack + viewport aspect core).
// Binary refs: FUN_004cc1f0 / FUN_004cbd20 / FUN_004cc2f0 / FUN_004cc350 /
//              FUN_004cc480 / FUN_004cc4d0 / FUN_004cc020
#include "RenderPipeline.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#define CHECK(condition)                                                     \
    do {                                                                     \
        if (!(condition)) {                                                  \
            std::fprintf(stderr, "FAIL at line %d: %s\n", __LINE__,          \
                         #condition);                                        \
            return 1;                                                        \
        }                                                                    \
    } while (0)

namespace
{
    bool Near(float a, float b)
    {
        return std::fabs(a - b) < 0.0005f;
    }

    // v' = v·M (sütun tabanlı: v' = right*vx + up*vy + at*vz + pos)
    void ApplyPoint(const CCMatrix *pM, float vx, float vy,
                    float *pOutX, float *pOutY)
    {
        *pOutX = pM->right[0] * vx + pM->up[0] * vy + pM->pos[0];
        *pOutY = pM->right[1] * vx + pM->up[1] * vy + pM->pos[1];
    }
}

int main()
{
    // ------------------------------------------------------------------
    // Reset — sub_4cc1f0: derinlik 0, birim matris, en-boy 1/1
    // ------------------------------------------------------------------
    CHECK(RenderPipeline_Reset());
    CHECK(RenderPipeline_GetStackDepth() == 0);
    const CCMatrix *pTop = RenderPipeline_GetTopMatrix();
    CHECK(Near(pTop->right[0], 1.0f) && Near(pTop->up[1], 1.0f) &&
          Near(pTop->at[2], 1.0f));
    CHECK(Near(pTop->pos[0], 0.0f) && Near(pTop->pos[1], 0.0f));
    CHECK((pTop->flags & 0x20003) == 0x20003); // dword_66d610 maskesi
    float ax = 0.0f, ay = 0.0f;
    RenderPipeline_GetAspect(&ax, &ay);
    CHECK(Near(ax, 1.0f) && Near(ay, 1.0f));

    // ------------------------------------------------------------------
    // Push — sub_4cc2f0: ebeveyn kopyası; taşma 31'de durur
    // ------------------------------------------------------------------
    CHECK(RenderPipeline_MatrixPush());
    CHECK(RenderPipeline_GetStackDepth() == 1);
    pTop = RenderPipeline_GetTopMatrix();
    CHECK(Near(pTop->right[0], 1.0f)); // kopya birim
    for (int i = 0; i < 30; ++i) {
        CHECK(RenderPipeline_MatrixPush()); // toplam 31 push
    }
    CHECK(RenderPipeline_GetStackDepth() == 31);
    CHECK(!RenderPipeline_MatrixPush()); // binary: cmp 1Fh → taşma
    CHECK(RenderPipeline_GetStackDepth() == 31);
    CHECK(!RenderPipeline_MatrixPushCopy(0)); // kopya da taşar

    // ------------------------------------------------------------------
    // DrawText sırası — translate → scale (sub_4ccaf0):
    // translate(10,20) birimde → pos=(10,20); scale(2,3) → tabanlar
    // ölçeklenir, pos sabit; v(1,1) → (12,23)
    // ------------------------------------------------------------------
    CHECK(RenderPipeline_Reset());
    CHECK(RenderPipeline_MatrixTranslateXY(10.0f, 20.0f));
    CHECK(RenderPipeline_MatrixScaleXY(2.0f, 3.0f));
    pTop = RenderPipeline_GetTopMatrix();
    CHECK(Near(pTop->right[0], 2.0f) && Near(pTop->up[1], 3.0f));
    CHECK(Near(pTop->pos[0], 10.0f) && Near(pTop->pos[1], 20.0f));
    float px = 0.0f, py = 0.0f;
    ApplyPoint(pTop, 1.0f, 1.0f, &px, &py);
    CHECK(Near(px, 12.0f) && Near(py, 23.0f));

    // ters sıra — scale → translate: öteleme ölçekten geçirilir
    // (RwMatrixTranslate semantiği): pos = (10×2, 20×3) = (20,60);
    // v(1,1) → (22,63)
    CHECK(RenderPipeline_Reset());
    CHECK(RenderPipeline_MatrixScaleXY(2.0f, 3.0f));
    CHECK(RenderPipeline_MatrixTranslateXY(10.0f, 20.0f));
    pTop = RenderPipeline_GetTopMatrix();
    CHECK(Near(pTop->pos[0], 20.0f) && Near(pTop->pos[1], 60.0f));
    ApplyPoint(pTop, 1.0f, 1.0f, &px, &py);
    CHECK(Near(px, 22.0f) && Near(py, 63.0f));

    // ------------------------------------------------------------------
    // MatrixIdentity — sub_4cbd20: üst operasyonları sıfırla
    // ------------------------------------------------------------------
    CHECK(RenderPipeline_MatrixIdentity());
    pTop = RenderPipeline_GetTopMatrix();
    CHECK(Near(pTop->right[0], 1.0f) && Near(pTop->up[1], 1.0f) &&
          Near(pTop->pos[0], 0.0f));
    CHECK((pTop->flags & 0x20003) == 0x20003);

    // ------------------------------------------------------------------
    // PushCopy — sub_4cc350: yeni üst = src × ebeveyn (ebeveyn önce)
    // src = scale(4,5); ebeveyn = translate(10,20) → pos = (40,100)
    // ------------------------------------------------------------------
    CHECK(RenderPipeline_Reset());
    CHECK(RenderPipeline_MatrixTranslateXY(10.0f, 20.0f)); // ebeveyn
    CCMatrix src;
    src.right[0] = 4.0f; src.right[1] = 0.0f; src.right[2] = 0.0f;
    src.up[0] = 0.0f; src.up[1] = 5.0f; src.up[2] = 0.0f;
    src.at[0] = 0.0f; src.at[1] = 0.0f; src.at[2] = 1.0f;
    src.pos[0] = 0.0f; src.pos[1] = 0.0f; src.pos[2] = 0.0f;
    src.flags = 0x20003;
    src.pad0 = src.pad1 = src.pad2 = 0;
    CHECK(RenderPipeline_MatrixPushCopy(&src));
    CHECK(RenderPipeline_GetStackDepth() == 1);
    pTop = RenderPipeline_GetTopMatrix();
    CHECK(Near(pTop->right[0], 4.0f) && Near(pTop->up[1], 5.0f));
    CHECK(Near(pTop->pos[0], 40.0f) && Near(pTop->pos[1], 100.0f));
    // NULL src → ebeveyn kopyası (sub_4cc350 arg_0==0)
    CHECK(RenderPipeline_MatrixPushCopy(0));
    pTop = RenderPipeline_GetTopMatrix();
    CHECK(Near(pTop->right[0], 4.0f) && Near(pTop->pos[0], 40.0f));

    // ------------------------------------------------------------------
    // SetViewWindow — sub_4cc020: en-boy (büyük/küçük → diğeri 1.0)
    // 640×480, win (320,240): 320>=240 → x=4/3, y=1
    // ------------------------------------------------------------------
    CHECK(RenderPipeline_SetViewWindow(640.0f, 480.0f, 320.0f, 240.0f));
    RenderPipeline_GetAspect(&ax, &ay);
    CHECK(Near(ax, 320.0f / 240.0f) && Near(ay, 1.0f));
    // dikey baskınlık: win (100,200) → x=1, y=2
    CHECK(RenderPipeline_SetViewWindow(640.0f, 480.0f, 100.0f, 200.0f));
    RenderPipeline_GetAspect(&ax, &ay);
    CHECK(Near(ax, 1.0f) && Near(ay, 200.0f / 100.0f));
    // sıfır pencere → 1.0 (fdivs korumalı port)
    CHECK(RenderPipeline_SetViewWindow(640.0f, 480.0f, 0.0f, 0.0f));
    RenderPipeline_GetAspect(&ax, &ay);
    CHECK(Near(ax, 1.0f) && Near(ay, 1.0f));

    // ------------------------------------------------------------------
    // SetViewWindowFromCamera — sub_4cc020 tam yolu (kamera bağlamı)
    // ------------------------------------------------------------------
    float wid = 0.0f, hei = 0.0f, wx2 = 0.0f, wy2 = 0.0f;
    float offX = 0.0f, offY = 0.0f;
    CCMatrix frame;
    std::memset(&frame, 0, sizeof(frame));
    frame.right[0] = frame.up[1] = frame.at[2] = 1.0f;
    frame.pos[0] = 10.0f;
    frame.pos[1] = 20.0f;
    frame.pos[2] = 30.0f;

    RCCCameraView cam;
    cam.projFlag = 2; // ortografik → zScale 1.0
    cam.rasterWidth = 640;
    cam.rasterHeight = 480;
    cam.viewWindowX = 1.0f; // ×2 → 2.0
    cam.viewWindowY = 0.5f; // ×2 → 1.0
    cam.pFrameMatrix = &frame;

    CHECK(RenderPipeline_SetViewWindowFromCamera(&cam));
    CHECK(RenderPipeline_GetCamera() == &cam);
    RenderPipeline_GetViewWindow(&wid, &hei, &wx2, &wy2);
    CHECK(Near(wid, 640.0f) && Near(hei, 480.0f));
    CHECK(Near(wx2, 2.0f) && Near(wy2, 1.0f));
    RenderPipeline_GetViewParams(&offX, &offY, &ax, &ay);
    CHECK(Near(offX, 0.0f) && Near(offY, 0.0f));
    CHECK(Near(ax, 2.0f) && Near(ay, 1.0f)); // winX2 2>=1 → x=2/1

    // NULL kamera → aktif önbellek temiz, FALSE
    CHECK(!RenderPipeline_SetViewWindowFromCamera(0));
    CHECK(RenderPipeline_GetCamera() == 0);
    CHECK(RenderPipeline_SetViewWindowFromCamera(&cam));
    // raster NULL → FALSE, aktif kamera KORUNUR (loc_4cc109)
    RCCCameraView bad = cam;
    bad.rasterWidth = 0;
    CHECK(!RenderPipeline_SetViewWindowFromCamera(&bad));
    CHECK(RenderPipeline_GetCamera() == &cam);

    // ------------------------------------------------------------------
    // BuildViewMatrix — sub_4cc520: ölçek matrisi + tersi + frame çarpımı
    // temiz yığın (Reset + kamera); w=min(2,1)=1; ölçek(-1,1,1);
    // translate(1,-0.5,0)
    // ------------------------------------------------------------------
    CHECK(RenderPipeline_Reset());
    CHECK(RenderPipeline_SetViewWindowFromCamera(&cam));
    const CCMatrix *pVS = RenderPipeline_BuildViewMatrix();
    CHECK(pVS != 0);
    const CCMatrix *pInv = RenderPipeline_GetViewInverseMatrix();
    const CCMatrix *pScale = RenderPipeline_GetViewScaleMatrix();
    CHECK(Near(pScale->right[0], -1.0f));
    CHECK(Near(pScale->up[1], 1.0f) && Near(pScale->at[2], 1.0f));
    CHECK(Near(pScale->pos[0], 1.0f) && Near(pScale->pos[1], -0.5f));
    // ters: diag(-1,1,1); pos' = -(inv×pos) = (1, 0.5, 0)
    CHECK(Near(pInv->right[0], -1.0f) && Near(pInv->up[1], 1.0f));
    CHECK(Near(pInv->pos[0], 1.0f) && Near(pInv->pos[1], 0.5f));
    // sonuç = ölçek × frame: pos = ölçek.3×3×frame.pos + ölçek.pos
    CHECK(Near(pVS->right[0], -1.0f));
    CHECK(Near(pVS->pos[0], -9.0f) && Near(pVS->pos[1], 19.5f) &&
          Near(pVS->pos[2], 30.0f));

    // önbellek: geçerliyken ölçek yeniden kurulmaz (statik adres sabit);
    // geçersizlik (MatrixTranslateXY) → yeniden kurulum
    CHECK(RenderPipeline_BuildViewMatrix() == pVS);
    CHECK(RenderPipeline_MatrixTranslateXY(3.0f, 4.0f));
    const CCMatrix *pVS2 = RenderPipeline_BuildViewMatrix();
    CHECK(pVS2 == pVS); // sonuç tamponu aynı
    // yığın üst'ü artık kaydırılmış — ölçek matrisi değişmiş olmalı
    // (translate kök'te değil üst'te; derinlik 0 → üst = kök'ten bağımsız
    //  yine de ölçek = üst×kök → pos kayar)
    CHECK(!Near(pScale->pos[0], 1.0f) || !Near(pScale->pos[1], -0.5f));

    // kamera/kare matrisi yoksa ölçek döner (port kılavuzu — binary
    // kilitlenirdi)
    RCCCameraView noFrame = cam;
    noFrame.pFrameMatrix = 0;
    CHECK(RenderPipeline_SetViewWindowFromCamera(&noFrame));
    CHECK(RenderPipeline_BuildViewMatrix() ==
          RenderPipeline_GetViewScaleMatrix());

    // ------------------------------------------------------------------
    // SetContext — sub_4cc130 (dword_66d79c bağlam ptr)
    // ------------------------------------------------------------------
    int ctxObj = 0;
    CHECK(RenderPipeline_SetContext(&ctxObj));
    CHECK(RenderPipeline_GetContext() == &ctxObj);
    RenderPipeline_ClearContext();
    CHECK(RenderPipeline_GetContext() == 0);

    std::printf("renderpipeline_test: all checks passed\n");
    return 0;
}

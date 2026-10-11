// ============================================================================
// CannonCruise - Rt2dMaestro gerçek implementasyonu (Rt2dMaestro.cpp)
//
// Eski: rwstubs.cpp no-op (Rt2dMaestroDestroy/Update/Render) — silindi.
// Yeni: parse edilmiş stream + seçim durumu + Im2D placeholder render.
// ============================================================================

#include "Rt2dMaestro.h"
#include "RenderPlatform.h"
#include "ShowText.h"

#include <rw.h> // RwIm2DVertexSetScreenX — C API (RenderPlatform.cpp ile aynı)

#include <cstring>

// NOT: Im2D çizimleri piksel koordinatlı olmalı (CShowText::Render deseni).
// Eski matris tabanlı DrawQuad view-window (-1..1) üretiyordu → ~1 piksel
// köşede çiziliyordu (görünmez). Artık RenderPlatform_DrawOverlayQuad kullanılıyor.

Rt2dMaestro *Rt2dMaestroCreate(void)
{
    return nullptr; // streamsiz create yok (binary da stream ister)
}

Rt2dMaestro *Rt2dMaestroCreateFromMemory(const void *pData, RwUInt32 len)
{
    if (pData == nullptr) {
        return nullptr;
    }
    Rt2dMaestro *p = new Rt2dMaestro;
    std::memset(p, 0, sizeof(*p));
    if (Rt2dMaestro_Inspect(pData, len, &p->info) != TRUE) {
        delete p;
        return nullptr;
    }
    // komut indekslerini topla (menü öğeleri)
    for (RwUInt32 i = 0; i < p->info.nodeCount; ++i) {
        if (p->info.nodes[i].type == rwrt2dNODE_COMMAND) {
            p->commands[p->commandCount++] = (RwInt32)i;
        }
    }
    p->selection = 0;
    p->time = 0.0f;
    return p;
}

RwBool Rt2dMaestroDestroy(Rt2dMaestro *maestro)
{
    delete maestro;
    return TRUE;
}

void Rt2dMaestroUpdate(Rt2dMaestro *maestro, RwReal deltaTime)
{
    if (maestro != nullptr) {
        maestro->time += deltaTime;
    }
}

// Im2D overlay: koyu zemin + başlık + komut listesi (gömülü 5x7 bitmap
// font ile okunur etiketler — gerçek menü görselleri {GUID} txd/dff yok).
// Tüm çizim PİKSEL koordinatlı (RenderPlatform_BeginOverlay2D altında).
void Rt2dMaestroRender(Rt2dMaestro *maestro)
{
    if (maestro == nullptr) {
        return;
    }
    // ekran pikseli — RenderPlatform_Initialize(1280, 720)
    const RwReal kScreenW = 1280.0f;
    const RwReal kScreenH = 720.0f;
    const int scale = 3;

    // Im2D render state'leri —3D sahne derinlik yazdı; ZTEST kapalı yoksa elenir
    RenderPlatform_BeginOverlay2D();

    // zemin (koyu mavi-siyah)
    RenderPlatform_DrawOverlayQuad(0.0f, 0.0f, kScreenW, kScreenH,
                                   0xFF0C1420);
    // başlık şeridi
    RenderPlatform_DrawOverlayQuad(kScreenW * 0.025f, kScreenH * 0.045f,
                                   kScreenW * 0.95f, kScreenH * 0.12f,
                                   0xFF204068);

    // başlık metni
    ShowText_DrawBitmapText("CANNONCRUISE MAINMENU", kScreenW * 0.05f,
                            kScreenH * 0.06f, scale, 220, 230, 255, 255);

    // komut listesi — seçim çubuğu + etiket
    const RwInt32 kVisible = 12; // ekrana sığan satır sayısı
    const RwInt32 total = maestro->commandCount;
    const RwInt32 sel = (total > 0) ? maestro->selection : 0;
    RwInt32 first = 0;
    if (total > kVisible) {
        first = sel - kVisible / 2;
        if (first < 0) {
            first = 0;
        }
        if (first > total - kVisible) {
            first = total - kVisible;
        }
    }
    const RwReal rowH = kScreenH * 0.055f;
    const RwReal listX = kScreenW * 0.08f;
    const RwReal listY = kScreenH * 0.22f;
    for (RwInt32 i = 0; i < kVisible && first + i < total; ++i) {
        const RwInt32 ci = first + i;
        const RwReal y = listY + (RwReal)i * rowH;
        const RwBool active = (ci == sel) ? TRUE : FALSE;
        // seçim/etiket zemini (piksel koordinatlı satır bandı)
        RenderPlatform_DrawOverlayQuad(listX, y - rowH * 0.1f,
                                       kScreenW * 0.84f, rowH * 0.7f,
                                       active ? 0xFFC89030u : 0xFF182838u);
        const char *name =
            maestro->info.nodes[maestro->commands[ci]].name;
        ShowText_DrawBitmapText(name, listX + kScreenW * 0.02f,
                                y, scale,
                                active ? 20 : 200,
                                active ? 20 : 210,
                                active ? 20 : 230, 255);
    }
    // toplam / ipucu
    char status[96];
    std::snprintf(status, sizeof(status),
                  "SELECTED: %s   [%d/%d]  W/S=MOVE ENTER=OK ESC=QUIT",
                  (total > 0)
                      ? maestro->info.nodes[maestro->commands[sel]].name
                      : "-",
                  (int)sel + 1, (int)total);
    ShowText_DrawBitmapText(status, kScreenW * 0.05f, kScreenH * 0.93f, 2,
                            180, 190, 200, 255);

    RenderPlatform_EndOverlay2D();
}

RwBool Rt2dMaestroSelectNext(Rt2dMaestro *maestro)
{
    if (maestro == nullptr || maestro->commandCount == 0) {
        return FALSE;
    }
    maestro->selection = (maestro->selection + 1) % maestro->commandCount;
    return TRUE;
}

RwBool Rt2dMaestroSelectPrev(Rt2dMaestro *maestro)
{
    if (maestro == nullptr || maestro->commandCount == 0) {
        return FALSE;
    }
    maestro->selection =
        (maestro->selection + maestro->commandCount - 1) % maestro->commandCount;
    return TRUE;
}

const char *Rt2dMaestroGetSelectedCommand(const Rt2dMaestro *maestro)
{
    if (maestro == nullptr || maestro->commandCount == 0) {
        return nullptr;
    }
    return maestro->info.nodes[maestro->commands[maestro->selection]].name;
}

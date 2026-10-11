// ============================================================================
// CannonCruise - Render Pipeline bootstrap (renderpipeline.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\startup\renderpipeline.cpp
// Tersine mühendislik ile CC_ida.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0041c190 — RenderPipelineInit    (startup init zinciri, "Adım 14")
//   FUN_0041c270 — RenderPipelineShutdown (startup shutdown zinciri, +6)
//   FUN_0041a640 — SetLightAmbient       (startup init zinciri, "Adım 13")
//
// Not: Dosya bağlamı yalnızca startup.cpp'deki çağrıdan biliniyor.
//      init gövdesindeki alt-sistem çağrı zincirinin tamamı aşağıda
//      adresleriyle listelenmiştir; bulunmayan hedefler Faz 4.4b'de
//      restorasyon bekliyor (çünkü ilgili alt sistemler kod tabanında yok).
// ============================================================================

#include "rwcore.h"
#include "eventhandler/ceventhandler.h"
#include "attributehandler/Allocation Policies/clinearmemorymanager.h"
#include "FontSystem.h"
#include "RenderPipeline.h"

namespace
{
    // dword_63600C — "iMsgStartSystem" kayıtlı olay kimliği (RenderPipelineInit kaydeder,
    //                  RenderPipelineShutdown siler — binary: loc_40BF90 / loc_40CC60)
    RWS::CEventId s_iMsgStartSystem = 0;

    // dword_636004 — "iMsgStopSystem" kayıtlı olay kimliği
    RWS::CEventId s_iMsgStopSystem = 0;

    // dword_635608 — "iMsgDeleteEventHandler" (sub_40BF10 kaydeder,
    //                  sub_40BF60 siler)
    RWS::CEventId s_iMsgDeleteEventHandler = 0;

    // unk_635600 — "iMsgDeleteEntity"
    RWS::CEventId s_iMsgDeleteEntity = 0;

    // dword_6355F8 — "iMsgOnDeleteEntity" (format: "CEventHandler*")
    RWS::CEventId s_iMsgOnDeleteEntity = 0;

    // dword_635644 — pipeline bildirim/zamanlayıcı geri çağırısı
    // Binary init sonunda: mov dword_635644, offset sub_41BFA0
    // Binary shutdown'da : mov dword_635644, 0
    // FUN_0041BFA0 gövdesi restor edilmediği için yer tutucu atanır (Faz 4.4b).
    typedef void (*PipelineCallback)();
    void PipelineCallbackStub()
    {
        // TODO(Faz 4.4b): FUN_0041BFA0 — pipeline geri çağrısı gövdesi
    }
    PipelineCallback s_pPipelineCallback = 0;
}

// ============================================================================
// RenderPipelineInit (FUN_0041C190)
// Binary Adresi: 0x0041c190
// XREF: sub_41A680 (startup init): 0041ab5d — startup.cpp "Adım 14"
// Argümanlar: arg_0 = width, arg_4 = height, arg_8 = flags (startup extern'i)
//
// Binary çağrı zinciri (hepsi tek tek, orijinal sırayla):
//   1. loc_429360(width, height, flags) → sub_41CE00(result)
//      — render hedefi / framebuffer yapısı
//   2. sub_41CDF0() → sub_4CC140(x) → sub_4D6220()
//   3. sub_40BF10(0, 0) — framework mesaj kayıtları:
//        "iMsgDeleteEventHandler" (unk_635608)
//        "iMsgDeleteEntity"       (unk_635600)
//        "iMsgOnDeleteEntity"     (dword_6355F8, format "CEventHandler*")
//   4. sub_41C650, sub_40F5C0, sub_414E60 (= CLinearMemoryManager::Delete)
//   5. dword_635644 = sub_41BFA0 (tick geri çağırısı)
//   6. sub_40F880, sub_408360→sub_40A070, sub_407DD0, sub_4082D0,
//      sub_429C50, sub_41BA60, sub_41BB90, sub_41B770, sub_41CE10,
//      nullsub_11, sub_4D9E40, sub_41CEB0 ×2, nullsub_11, sub_41C0E0
//   7. loc_40BF90(dword_63600C, "iMsgStartSystem", 0)
//      loc_40BF90(dword_636004, "iMsgStopSystem",  0)
//
// Restorasyon: mevcut API ile modellenebilen adımlar (3, 4, 5, 7) çalıştırılır;
// bulunmayan alt sistem hedefleri TODO(Faz 4.4b) olarak korunur.
// ============================================================================
void RenderPipelineInit(RwUInt32 width, RwUInt32 height, RwUInt32 flags)
{
    (void)width; (void)height; (void)flags;

    // Adım 2 — sub_41CDF0() → sub_4CC140(x) → sub_4D6220():
    //   sub_4CC140 zinciri orijinal sırayla: sub_4D5450, sub_4D0E70,
    //   sub_4CEFD0 (=FontSystemAllocBuffers), sub_4CD660 (=FontSystemInit),
    //   sub_4D41C0, sub_4CC1F0, sub_4CC020.
    //   sub_4D5450/4D0E70/4D41C0 motor akış kayıtları (sub_50dbf0 +
    //   dword_65d7a4 vtable tahsisi) — portta havuz tahsisi karşılar,
    //   kayıt no-op → TODO(Faz 4.4b motor). Taşınabilir state
    //   (sub_4CC1F0) RenderPipeline_Reset ile restor edildi; sub_4CC020
    //   kamera nesnesi gerektirir → en-boy Reset'te 1.0 kalır (binary
    //   kamera'sız aynı).
    FontSystemAllocBuffers(); // sub_4CEFD0 — glyph vertex tamponu (0x2400)
    FontSystemInit();         // sub_4CD660 — yol + çeyrek-dörtgen tablosu
    RenderPipeline_Reset();   // sub_4CC1F0 — matris yığını + Im2D state

    // Adım 3 — framework mesajları (binary: sub_40BF10(0, 0)):
    //   "iMsgDeleteEventHandler" / "iMsgDeleteEntity" / "iMsgOnDeleteEntity"
    //   (sonuncu format "CEventHandler*"). sub_40E1E0(0) ve loc_40E9E0(0)
    //   ön çağrıları alt sistem gerektirir → TODO.
    s_iMsgDeleteEventHandler =
        RWS::CEventHandler::RegisterMsg("iMsgDeleteEventHandler");
    s_iMsgDeleteEntity =
        RWS::CEventHandler::RegisterMsg("iMsgDeleteEntity");
    s_iMsgOnDeleteEntity =
        RWS::CEventHandler::RegisterMsg("iMsgOnDeleteEntity", "CEventHandler*");

    // Adım 4 — CLinearMemoryManager::Delete (binary: sub_414E60)
    // Pipeline başlangıcında lineer bellek havuzlarını sıfırlar.
    RWS::CLinearMemoryManager::Delete();

    // Adım 5 — tick geri çağırısı (binary: dword_635644 = sub_41BFA0)
    s_pPipelineCallback = &PipelineCallbackStub;

    // Adım 7 — pipeline mesajları (binary: loc_40BF90)
    s_iMsgStartSystem = RWS::CEventHandler::RegisterMsg("iMsgStartSystem");
    s_iMsgStopSystem  = RWS::CEventHandler::RegisterMsg("iMsgStopSystem");

    // TODO(Faz 4.4b) — adım 1, 2, 6 alt sistem zinciri:
    //   loc_429360 / sub_41CE00 / sub_41CDF0 / sub_4D6220
    //   sub_4CC140'in motor üyeleri: sub_4D5450 / sub_4D0E70 / sub_4D41C0
    //   (akış kayıtları + freelist); kamera bağlamı taşınabilir yol
    //   (RenderPipeline_SetViewWindowFromCamera / BuildViewMatrix) ile
    //   portlandı — vtable kancası (sub_4CC110) motor tarafı kalır
    //   (sub_4CC1F0 taşınabilir reset = RenderPipeline_Reset yukarıda)
    //   (sub_40BF10 ön çağrıları: sub_40E1E0 / loc_40E9E0)
    //   sub_41C650 / sub_40F5C0 / sub_40F880 / sub_408360→sub_40A070 /
    //   sub_407DD0 / sub_4082D0 / sub_429C50 / sub_41BA60 / sub_41BB90 /
    //   sub_41B770 / sub_41CE10 / sub_4D9E40 / sub_41CEB0 / sub_41C0E0
}

// ============================================================================
// RenderPipelineShutdown (FUN_0041C270)
// Binary Adresi: 0x0041c270
// XREF: sub_41AB90 (startup shutdown): 0041ab06 — startup.cpp shutdown zinciri
//
// Binary çağrı zinciri:
//   1. loc_40CC60(dword_636004) → iMsgStopSystem kaydını sil
//      loc_40CC60(dword_63600C) → iMsgStartSystem kaydını sil
//   2. sub_41C140, nullsub_11, sub_40D840(unk_635608)
//   3. sub_411CE0, sub_408360→sub_40A080, sub_4D9DF0, sub_41BBC0,
//      sub_41BB10, nullsub_11, sub_41CE70, sub_429C70, sub_40F910, sub_411C10
//   4. dword_635644 = 0 (tick geri çağırısını kaldır)
//   5. sub_407320, sub_408320, sub_407DE0, sub_40F5E0
//   6. CLinearMemoryManager havuz döngüsü: Instance() → her havuza
//      sub_415200 (DestroyPool) — kod tabanında Delete() ile modellendi
//   7. sub_40BF60 — framework mesaj kayıtlarını sil
//   8. sub_4229A0, sub_4D6330, __ioterm, sub_41CDF0 → sub_429070,
//      sub_41CE00(0) — render hedefini bırak
// ============================================================================
void RenderPipelineShutdown(void)
{
    // Adım 1 — pipeline mesaj kayıtlarını sil (binary: loc_40CC60 → UnRegisterMsg)
    if (s_iMsgStopSystem)
    {
        RWS::CEventHandler::UnRegisterMsg(s_iMsgStopSystem);
        s_iMsgStopSystem = 0;
    }
    if (s_iMsgStartSystem)
    {
        RWS::CEventHandler::UnRegisterMsg(s_iMsgStartSystem);
        s_iMsgStartSystem = 0;
    }

    // Adım 4 — tick geri çağırısını kaldır (binary: dword_635644 = 0)
    s_pPipelineCallback = 0;

    // Adım 6 — lineer bellek havuzlarını serbest bırak.
    // Binary havuzları tek tek DestroyPool ile kaldırır ve manager'ı yaşatır;
    // kod tabanındaki en yakın karşılık Delete() (manager tembel yeniden kurulur).
    RWS::CLinearMemoryManager::Delete();

    // Adım 7 — framework mesaj kayıtlarını sil (binary: sub_40BF60 →
    // loc_40CC60 ×3; ardından sub_40EC70 / loc_40E470 abone temizliği TODO)
    if (s_iMsgDeleteEventHandler)
    {
        RWS::CEventHandler::UnRegisterMsg(s_iMsgDeleteEventHandler);
        s_iMsgDeleteEventHandler = 0;
    }
    if (s_iMsgDeleteEntity)
    {
        RWS::CEventHandler::UnRegisterMsg(s_iMsgDeleteEntity);
        s_iMsgDeleteEntity = 0;
    }
    if (s_iMsgOnDeleteEntity)
    {
        RWS::CEventHandler::UnRegisterMsg(s_iMsgOnDeleteEntity);
        s_iMsgOnDeleteEntity = 0;
    }

    // Adım 8 (kısmi) — font temizliği: binary __ioterm atexit zincirinde
    //   +62 sub_4CD580 (FontSystemTeardown — ad listesi/havuzlar/yol)
    //   +67 sub_4CEF90 (FontSystemShutdown — vertex tamponu)
    //   __ioterm kendisi restor edilmediği için çağrılar burada modellenir.
    FontSystemTeardown();
    FontSystemShutdown();

    // TODO(Faz 4.4b) — adım 2, 3, 5, 8 alt sistem zinciri:
    //   sub_41C140 / sub_40D840 (unk_635608 abone temizliği) / sub_411CE0 /
    //   sub_408360→sub_40A080 / sub_4D9DF0 / sub_41BBC0 / sub_41BB10 /
    //   sub_41CE70 / sub_429C70 / sub_40F910 / sub_411C10 / sub_407320 /
    //   sub_408320 / sub_407DE0 / sub_40F5E0 / sub_40EC70+loc_40E470
    //   (sub_40BF60 devamı) / sub_4229A0 / sub_4D6330 / __ioterm /
    //   sub_41CDF0 → sub_429070 / sub_41CE00(0)
}

// ============================================================================
// SetLightAmbient (FUN_0041A640)
// Binary Adresi: 0x0041a640
// XREF: sub_41A680 (startup init): 0041a6..+4CA — startup.cpp "Adım 13"
//
// Binary gövdesi:
//   1. sub_41A5A0() — ışık bağlamı üretir (sub_41A500 tanım + sub_50D2A0(3, 1, &cmd)
//      + sub_50D1D0(cmd, 0)). Bağlam null ise sessizce döner.
//   2. [bağlam+0x18] &= ~0x00000002 — mevcut ışık/bağlantı bayrağını temizle
//      (binary: and 0FFFFFFFDh)
//   3. sub_50D2A0(3, 1, &cmd) + sub_50D1D0(cmd, 0) — ambient ışık komutu
//      (type = 3 → rpLIGHTAMBIENT ailesi) uygulanır.
//
// sub_41A5A0 / sub_41A500 / sub_50D2A0 / sub_50D1D0 RW motor komut
// altyapısına (engine command object, dword_65D7A4 vtable +0x144h tahsisi)
// bağlıdır; kod tabanında karşılığı yok → TODO(Faz 4.4b).
// ============================================================================
void SetLightAmbient(void)
{
    // TODO(Faz 4.4b) — bağlam üret (sub_41A5A0), bayrağı temizle
    // ([bağlam+0x18] &= ~2) ve sub_50D2A0(3, 1)/sub_50D1D0 ile ambient
    // ışığı sahneye ekle. Bağlam restor edilene kadar bilinçli no-op.
}

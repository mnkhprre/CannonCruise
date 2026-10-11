// ============================================================================
// CannonCruise - Havok sistem bootstrap (havokstart.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\startup\havokstart.cpp
// Tersine mühendislik ile CC_ida.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00434fb0 — HavokInit        (startup init zinciri, "Adım 1")
//   FUN_004356f0 — TextureCacheShutdown (startup shutdown zinciri, +1F)
//
// Not: TextureCacheShutdown bu dosyadadır çünkü binary'de HavokInit ile
//      AYNI global yaşam döngüsünü paylaşırlar:
//        byte_660934  — sistem bayrağı (HavokInit kurar, shutdown temizler)
//        dword_660920 — aktif hafıza context'i (Havok singleton'ı)
//        dword_660938 / dword_66093c / dword_660940 — HavokInit'in yarattığı
//                          3 refcounted alt sistem nesnesi (shutdown bırakır)
//      Dosya bağlamı: fonksiyon içi assert path string'i yok (yalnızca
//      startup.cpp çağrı XREF'i var).
// ============================================================================

#include "rwcore.h"
#include <cstdlib>
#include <cstring>

namespace
{
    // -------------------------------------------------------------------------
    // byte_660934 — sistem yaşam döngüsü bayrağı
    // -------------------------------------------------------------------------
    bool s_havokInited = false;

    // -------------------------------------------------------------------------
    // dword_660920 — aktif hafıza context'i (sub_52DCA0 ile takas edilir)
    // sub_435810 ctor sözleşmesi:
    //   [+0x00] vtable (off_5EF898) — sınıf metotları Faz 4.4b
    //   [+0x04] refcount = 1
    //   [+0x08, +0x0C, +0x10] = 0
    //   [+0x14] = -1
    //   [+0x20 .. +0x4028) = 0 (0x1002 dword arena tablosu, binary: rep stosd)
    //   sub_52DC80(this, align16(+0x2F), 0x4000) — arena kurulumu (TODO)
    //   Toplam tahsis: 0x4030 bayt (sub_4053F0(4030h, 2))
    // -------------------------------------------------------------------------
    struct HavokContext
    {
        void*    vtable;          // +0x00
        RwUInt32 refCount;        // +0x04
        RwUInt32 field_08;        // +0x08
        RwUInt32 field_0C;        // +0x0C
        RwUInt32 field_10;        // +0x10
        RwInt32  field_14;        // +0x14 = -1
        RwUInt32 pad_18;          // +0x18
        RwUInt32 pad_1C;          // +0x1C
        RwUInt32 arena[0x1002];   // +0x20 .. +0x4028
        RwUInt32 padTail[2];      // +0x4028 .. +0x4030 (tahsis tam boyu)
    };

    HavokContext* s_pMemoryContext = 0;   // dword_660920

    // -------------------------------------------------------------------------
    // Refcounted alt sistem nesneleri (binary: vtable+0x10h tahsisi, 0x12 + type)
    //   dword_660940 — type = 0x08, vtable off_5EF848
    //   dword_66093C — type = 0x20, vtable off_5EF83C (ctor sub_4357A0 sonrası
    //                  off_5EF830'a takas edilir; +0x08/+0x14 zincirleri
    //                  sub_52DCE0 ile kurulur — TODO Faz 4.4b)
    //   dword_660938 — type = 0x08, vtable off_5EF808
    // Binary'de refcount +0x06'da (word), sıfıra inince vtable[0](1) çağrısı
    // ile yok edilir.
    // -------------------------------------------------------------------------
    struct HavokRefObject
    {
        void*    vtable;      // +0x00
        RwUInt16 type;        // +0x04
        RwUInt16 refCount;    // +0x06
        void*    links[6];    // +0x08 .. +0x1C
    };

    HavokRefObject* s_pSystem38 = 0;   // dword_660938
    HavokRefObject* s_pSystem3C = 0;   // dword_66093C
    HavokRefObject* s_pSystem40 = 0;   // dword_660940

    // sub_4053F0(size, flags) — MSVC operator new karşılığı (binary yorumu)
    void* SysAlloc(size_t size)
    {
        return calloc(1, size);
    }

    // Alt sistem nesnesi oluştur (binary: context vtable[+0x10h](0x12, type))
    HavokRefObject* CreateRefObject(RwUInt16 type, void* vtable)
    {
        HavokRefObject* p = static_cast<HavokRefObject*>(SysAlloc(sizeof(HavokRefObject)));
        if (!p)
            return 0;
        p->vtable = vtable;
        p->type = type;
        p->refCount = 1;
        return p;
    }

    // Binary refcount deseni: dec word [+0x06]; sıfır → vtable[0](1) / free
    void ReleaseRefObject(HavokRefObject*& pObject)
    {
        if (!pObject)
            return;
        if (pObject->refCount > 0)
        {
            --pObject->refCount;
            if (pObject->refCount == 0)
            {
                // vtable[0](1) — yok edici (TODO(Faz 4.4b): sınıf dtor'ları)
                free(pObject);
            }
        }
        pObject = 0;
    }

    // sub_52DCA0(pNew) — aktif hafıza context'ini takas eder:
    //   var olan varsa refcount-1 yap; 0'a indiyse vtable[+0x20h](1) ile yok et;
    //   sonra dword_660920 = pNew.
    void SetActiveMemoryContext(HavokContext* pNew)
    {
        if (s_pMemoryContext)
        {
            if (s_pMemoryContext->refCount > 0)
            {
                --s_pMemoryContext->refCount;
                if (s_pMemoryContext->refCount == 0)
                {
                    // vtable[+0x20h](1) — context yok edici (model: serbest bırak)
                    free(s_pMemoryContext);
                }
            }
        }
        s_pMemoryContext = pNew;
    }

    // sub_435810 — HavokContext ctor (binary contract, bkz. struct yorumu)
    HavokContext* CreateHavokContext()
    {
        HavokContext* p = static_cast<HavokContext*>(SysAlloc(0x4030));
        if (!p)
            return 0;
        p->vtable = 0;                       // off_5EF898 (TODO Faz 4.4b)
        p->refCount = 1;                     // [esi+4] = 1
        p->field_08 = 0;
        p->field_0C = 0;
        p->field_10 = 0;
        p->field_14 = -1;                    // [esi+14h] = -1
        memset(p->arena, 0, sizeof(p->arena));   // rep stosd (0x1002 dword)
        // TODO(Faz 4.4b): sub_52DC80(this, align16(+0x2F), 0x4000)
        return p;
    }
}

// ============================================================================
// HavokInit (FUN_00434FB0)
// Binary Adresi: 0x00434fb0
// XREF: sub_41A680 (startup init): 0041a6a5 — startup.cpp "Adım 1"
//
// Binary gövdesi (SEH sarmalayıcı atlandı):
//   1. byte_660934 != 0 → zaten kuruldu, doğrudan çıkış
//   2. singleton = sub_4053F0(4030h, 2) + sub_435810 ctor
//   3. sub_52DCA0(singleton) — aktif hafıza context'ine takas
//   4. dword_660940: type 8, vtable off_5EF848 (eski ref serbest bırakılır)
//   5. dword_66093C: type 0x20, vtable off_5EF83C → ctor sub_4357A0,
//      sub_52DCE0 zincirleri (+0x08, +0x14), vtable → off_5EF830
//   6. dword_660938: type 8, vtable off_5EF808
//   7. sub_52E600() — sonraki kurulum aşaması (inverse: sub_52E700)
//   8. byte_660934 = 1; çıkışta off_6229C4 çağrısı (atexit zinciri)
//
// Not: DAT_00635af4 (ms_pSingleton) bu fonksiyonda YAZILMAZ — startup
//      doğrulasın diye null kalır (startup assert: "ms_pSingleton == NULL
//      Failed", .\Singleton.h satır 0x33).
// ============================================================================
void HavokInit(void)
{
    if (s_havokInited)
        return;

    // Adım 2-3 — singleton + aktif context takası
    HavokContext* pContext = CreateHavokContext();
    if (!pContext)
        return;
    SetActiveMemoryContext(pContext);

    // Adım 4 — type 8 nesne (dword_660940)
    {
        HavokRefObject* pOld = s_pSystem40;
        s_pSystem40 = CreateRefObject(8, 0);          // vtable: off_5EF848
        ReleaseRefObject(pOld);
    }

    // Adım 5 — type 0x20 nesne (dword_66093C)
    // TODO(Faz 4.4b): ctor sub_4357A0, sub_52DCE0 zincirleri (+0x08/+0x14),
    // vtable takası off_5EF83C → off_5EF830, +0x10/+0x1C ref serbest bırakma
    {
        HavokRefObject* pOld = s_pSystem3C;
        s_pSystem3C = CreateRefObject(0x20, 0);       // vtable: off_5EF83C
        ReleaseRefObject(pOld);
    }

    // Adım 6 — type 8 nesne (dword_660938)
    {
        HavokRefObject* pOld = s_pSystem38;
        s_pSystem38 = CreateRefObject(8, 0);          // vtable: off_5EF808
        ReleaseRefObject(pOld);
    }

    // Adım 7 — TODO(Faz 4.4b): sub_52E600()
    s_havokInited = true;                             // byte_660934 = 1

    // Adım 8 — TODO(Faz 4.4b): off_6229C4 (atexit/temizlik zinciri kaydı)
}

// ============================================================================
// TextureCacheShutdown (FUN_004356F0)
// Binary Adresi: 0x004356f0
// XREF: sub_41AB90 (startup shutdown): 0041ab..+1F — startup.cpp shutdown zinciri
//
// Binary gövdesi:
//   1. byte_660934 == 0 → kurulum yok; doğrudan tail'e (off_6229C8)
//   2. sub_52E700() — HavokInit sub_52E600'nin tersi (TODO)
//   3. dword_660938 refcount-1; 0 → vtable[0](1); global = 0
//   4. dword_66093C için aynı
//   5. dword_660940 için aynı
//   6. sub_52DCA0(0) — aktif context'i null'a takas (eski context refcount
//      0'a inerse yok edilir → singleton burada yaşar/ölür)
//   7. byte_660934 = 0; tail: jmp off_6229C8 (global temizlik zinciri, TODO)
// ============================================================================
void TextureCacheShutdown(void)
{
    if (!s_havokInited)
        return;

    // Adım 2 — TODO(Faz 4.4b): sub_52E700()

    // Adım 3-5 — üç alt sistem nesnesini bırak (binary sırası: 38, 3C, 40)
    ReleaseRefObject(s_pSystem38);
    ReleaseRefObject(s_pSystem3C);
    ReleaseRefObject(s_pSystem40);

    // Adım 6 — aktif context'i null'a takas (binary: sub_52DCA0(0))
    SetActiveMemoryContext(0);

    s_havokInited = false;                            // byte_660934 = 0

    // Adım 7 — TODO(Faz 4.4b): off_6229C8 (global temizlik zinciri)
}

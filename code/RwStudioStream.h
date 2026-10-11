// ============================================================================
// CannonCruise - rwstudio Konteyner Okuyucu (RwStudioStream.h)
// Original path: D:\Projects\CannonCruisePC\code\RwStudioStream.h
//
// Streams/*.RWS = "rwstudio" (WaveBreaker asset pipeline) özel konteyneri.
// Dosya = üst seviye chunk dizisi (standart RW 12 bayt başlık, libid
// 0x1802FFFF = RW 3.6.0.2):
//   0x0716 = asset kaydı  — meta (typeName/rwId/sourcePath/GUID) + gömülü
//            standart RW chunk (ör. txd: type=0x23, libid=0x1803FFFF)
//   0x0704 = nesne/behaviour kaydı (CWb* sınıf adları + property listesi)
//   0x071C = dosya sınıf tablosu (manifest: class adı + sürüm)
// Binary bağları: CPhysicsResource::Load (sub_439460) wbID_STATICWORLD-
// COLLISION chunk'ını bu konteynerden okur; CClassFactory/CEventHandler
// 0x071C/0x0704 sınıf adlarıyla çalışır.
//
// String kuralları (hexdump ile doğrulandı):
//   - uzunluk önekli: [u32 len][len bayt; '\0' + 0xBF doldurma, 4 hizalı]
//   - manifest içi: null-terminated + 0xBF doldurma (uzunluk yok)
// ============================================================================

#ifndef RWSTUDIOSTREAM_H
#define RWSTUDIOSTREAM_H

#include "rwcore.h"

// Üst seviye chunk türleri (oyun plugin aralığı — MAKEPLUGINID benzeri)
#define rwstudioID_ASSETRECORD 0x0716
#define rwstudioID_OBJECTRECORD 0x0704
#define rwstudioID_MANIFEST 0x071C

// Gömülü türler (asset kaydı içindeki standart RW chunk type)
#define rwstudioEMBED_TEXDICT 0x0023 // rwID_TEXDICTIONARY (bu RW sürümü)

#define rwstudioMAX_NAME 64
#define rwstudioMAX_PATH 256
#define rwstudioMAX_CLASSES 64
#define rwstudioMAX_BEHAVIOURS 8
#define rwstudioMAX_PROPS 32

// 0x0704 nesne property şeması (Faz 4.6f — MAINMENU obj0/obj1/obj2
// hexdump): nesne = [u32 field0] + tekrarlayan segmentler/property'ler.
//   segment: [u32 segLen][u32 flags][içerik]
//     flags 0x20000000 = sınıf adı (behaviour başlangıcı)
//     flags 0x40000000 = GUID (16B)
//     flags 0x80000000 = sınıf adı tekrarı
//     flags 0x00000000 = string değer (ör. "Audio_MenuMusic")
//   property: [u32 size][u32 id][size-8 bayt değer]
//     size 12 = u32/float/inline-string; size 20 = 3 float (vec3)
struct RwStudioProperty {
    RwUInt32 id;
    RwUInt32 size;   // toplam kayıt (8 + değer baytı)
    RwUInt8 value[16];
};

struct RwStudioBehaviour {
    RwChar className[48];
    RwChar name[48]; // flags=0 string segment (yoksa boş)
    RwUInt8 guid[16];
    RwUInt32 propCount;
    RwStudioProperty props[rwstudioMAX_PROPS];
};

struct RwStudioObject {
    RwUInt32 objectIndex; // ilk u32 (tüm örneklerde 0 — anlam belirsiz)
    RwUInt32 behaviourCount;
    RwStudioBehaviour behaviours[rwstudioMAX_BEHAVIOURS];
};

struct RwStudioAsset {
    RwUInt32 chunkType;  // rwstudioID_*
    RwUInt32 chunkLength;

    // 0x0716 meta (diğer türlerde boş)
    RwChar typeName[rwstudioMAX_NAME];   // "Texture Dictionary" vb.
    RwChar rwIdName[rwstudioMAX_NAME];   // "rwID_TEXDICTIONARY" vb.
    RwChar sourcePath[rwstudioMAX_PATH]; // kaynak .txd/.dff/.col yolu
    RwUInt8 guid[16];

    // gömülü RW chunk (embeddedType==0 ise yok)
    RwUInt32 embeddedType;   // 0x23 = txd
    RwUInt32 embeddedLength; // embedded chunk toplam uzunluğu (header dahil)
    RwUInt32 embeddedLibId;  // 0x1803FFFF = RW 3.6.0.3

    // embed OLMAYAN resource payload'u (örn. wbID_STATICWORLDCOLLISION .col):
    // meta+resource/va/embA/embB sonrası chunk sonuna kadar olan baytlar.
    // embeddedType!=0 iken 0.
    RwUInt32 resourceLength;
};

struct RwStudioFile;

RwStudioFile *RwStudioFileOpen(const RwChar *pPath);
void RwStudioFileClose(RwStudioFile *pFile);

// Sıradaki üst seviye chunk'ı çıkarır (tüm türler). Akıştaki konum chunk
// sonundan sonrasına ayarlanır; gömülü payload okunacaksa ReadEmbedded
// ÇAĞRILMADAN ÖNCE çağrılmalıdır (payload chunk sonrasıdır; okunmazsa
// NextAsset_skip eder).
RwBool RwStudioFileNextAsset(RwStudioFile *pFile, RwStudioAsset *pAsset);

// Geçerli asset'in gömülü payload'ını okur (embeddedType!=0 iken).
// maxLen payload boyundan büyükse fazlası atlanır (akış hizası korunur).
RwUInt32 RwStudioAssetReadEmbedded(RwStudioFile *pFile,
                                   const RwStudioAsset *pAsset, void *pBuf,
                                   RwUInt32 maxLen);

// Geçerli asset'in embed OLMAYAN resource payload'unu okur
// (resourceLength>0 iken; bkz. STATICWORLDCOLLISION .col).
RwUInt32 RwStudioAssetReadResource(RwStudioFile *pFile,
                                   const RwStudioAsset *pAsset, void *pBuf,
                                   RwUInt32 maxLen);

// 0x071C manifest görüldükten sonra sınıf listesi (boşluk: 0).
RwUInt32 RwStudioFileGetClassCount(const RwStudioFile *pFile);
const RwChar *RwStudioFileGetClassName(const RwStudioFile *pFile,
                                       RwUInt32 index);

// NextAsset 0x0704 döndürdükten sonra geçerli nesne şeması (dosya içi
// tampon; sonraki NextAsset ezer). Diğer türlerde NULL.
const RwStudioObject *RwStudioFileGetObject(const RwStudioFile *pFile);

#endif // RWSTUDIOSTREAM_H

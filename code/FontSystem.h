#ifndef FONTSYSTEM_H
#define FONTSYSTEM_H

// ============================================================================
// CannonCruise - Font Metrics System (FontSystem.h)
// Original path: D:\Projects\CannonCruisePC\code\FontSystem.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004cefd0 — FontSystemAllocBuffers (glyph vertex tamponu + 0x78 havuz)
//   FUN_004cd660 — FontSystemInit         (yol, glif çeyrek-dörtgen tablosu)
//   FUN_004cd9a0 — FontSystemLoad         (fonts/*.met, METRICS1 ayrıştırıcı)
//   FUN_004cd4d0 — FontAlloc              (font nesnesi tahsisi)
//   FUN_004cd8c0 — FontFree               (font nesnesi serbest bırakma)
//   FUN_004cdbc0 — ParseMetrics1          (METRICS1 metin biçimi)
//   FUN_004ce870 — ParseMetrics3          (METRICS3 vektör kontur biçimi;
//                  begin/moveto/lineto/curveto/closepath/end — PostScript
//                  benzeri; çizim işleyicisi sub_4CCAF0, kontur raster
//                  sub_4D54C0)
//   FUN_004cc690 — FontSystem_LoadFromStream (binary stream font yükleyici
//                  — sub_491700 zinciri; tip baytı ≥1, doku/vektör iki yol)
//   FUN_004d11d0 — LoadContourFromStream (type-5 kontur kaydı: 0x10 başlık
//                  + 0x18'lik hazır stroke kayıtları; flags&4 → zincir)
//   FUN_004ce000 — FontFinalizeExtMap     (genişletilmiş kod haritası)
//   FUN_004cee90 — FontSystemGet          (ad önbelleği: istek → düğüm)
//   FUN_004cd580 — FontSystemTeardown     (__ioterm+62: tüm önbellek/havuz)
//   FUN_004cef90 — FontSystemShutdown     (__ioterm+67: vertex tamponu)
//
// Not: Orijinal .met varlıkları deposuz olduğu için birim testler sentetik
//      dosyalarla doğrular. METRICS2 (FUN_004ce0d0) analiz edildi — raster
//      piksel taramasına dayalı format, taşınabilir port bloklu (bkz.
//      ROADMAP.md); METRICS3 restor edildi (kontur verisi + ilerleme; kontur
//      rasterizasyonu sub_4D54C0 Faz 4.4b).
// ============================================================================

#include <rwcore.h>

/**
 * @brief Vektör kontur noktası — binary sub_4CFE80/4CFF40/4D0040 kayıtları.
 *
 * Nokta türleri (binary: [esi] dword): 0 = moveto (sub_4CFE80), 1 = lineto
 * (sub_4CFF40), 2 = curveto (sub_4D0040 — PostScript sırası x1 y1 x2 y2
 * x3 y3: x/y = birinci kontrol, cx1/cy1 = ikinci kontrol, cx2/cy2 = uç
 * nokta). Binary 0x18 bayt kayıt; port bezier'i ham tutar (rasterizasyon
 * sub_4D54C0 — Faz 4.4b). ex/ey ayrılmıştır (kullanılmıyor).
 */
struct CCFontPathPoint {
    RwUInt32 type;  ///< 0=moveto, 1=lineto, 2=curveto
    RwReal   x;     ///< moveto/lineto noktası; curveto: kontrol1 X
    RwReal   y;
    RwReal   cx1;   ///< curveto: kontrol2 X
    RwReal   cy1;
    RwReal   cx2;   ///< curveto: uç nokta X
    RwReal   cy2;
    RwReal   ex;    ///< ayrılmış (kullanılmıyor)
    RwReal   ey;
};

/**
 * @brief Vektör alt yol (subpath) — binary sub_4D0C90 kontur nesnesi.
 *
 * Binary her moveto (dizi doluysa) yeni bir kontur nesnesi başlatıp
 * +0x14 zincirine dizer; port aynı semantiği pNext ile modelleyen düz
 * dinamik dizi kullanır (engine havuz/array yerine malloc).
 */
struct CCFontContour {
    CCFontPathPoint *points;      ///< nokta dizisi
    RwInt32          pointCount;
    RwInt32          capacity;
    RwBool           closed;      ///< closepath (sub_4D0220) görüldü
    CCFontContour   *pNext;       ///< sonraki alt yol (moveto zinciri)
};

/**
 * @brief Bir glifin 32 baytlık metrik kaydı (binary: +0x00..+0x1F).
 *
 * CC_ida.asm sub_4CDBC0 glif satırı ("%i %i %i %i %i") alanları:
 * kod, sol, üst, sağ, alt piksel koordinatlarıdır; UV'ler doku
 * boyutlarına bölünerek (piksel merkezi +0.5) kaydedilir.
 */
struct CCFontGlyph {
    RwReal     aspect;    ///< +0x00 (sağ-sol)/(alt-üstü) en-boy oranı
    RwTexture *texture;   ///< +0x04 glif doku adayı (M1'de font doku [0])
    RwReal     u0;        ///< +0x08 sol UV
    RwReal     v0;        ///< +0x0C üst UV
    RwReal     u1;        ///< +0x10 sağ UV
    RwReal     v1;        ///< +0x14 alt UV
    void      *data;      ///< +0x18 kontur/kern verisi (M1 = 0; M3 =
                          ///<      CCFontContour* alt yol zinciri başı —
                          ///<      FontFree sub_4D0C20 ile serbest bırakır)
    RwUInt8    flags;     ///< +0x1C bayrak (M1 = 0)
    RwUInt8    pad[3];
};

/**
 * @brief Font nesnesi — binary boyutu 0x144 (324 bayt), sub_4CD4D0.
 *
 * Yerleşim binary ile birebir şemalanmıştır; sub_4CD4D0 alanı başlangıç
 * değerleri (refCount=1, sizeScale=1.0f, ascii[]=0xFFFF...) korunur.
 */
struct CCFont {
    RwUInt32   refCount;      ///< +0x00 (sub_4CD4D0: 1)
    RwReal     lineExtent;    ///< +0x04 son glifin (alt-üstü) değeri (piksel)
    RwReal     sizeScale;     ///< +0x08 .met boyutu / lineExtent
    RwReal     spacing;       ///< +0x0C glifler arası ilerleme boşluğu
                              ///<   (M1: 0 — sub_4CDBC0 satır sonunda yazar)
    RwUInt32   glyphFlags;    ///< +0x10 (M1: |= 2 — genişletilmiş kod var)
    RwTexture *textures[4];   ///< +0x14,+0x18,+0x1C,+0x20 (M1 yalnızca [0])
    RwUInt16   ascii[128];    ///< +0x24..+0x123 — 0..127 kod → glif indeksi
                              ///<   (boş = 0xFFFF; sub_4CD4D0 memset -1)
    RwInt32    extMin;        ///< +0x124 genişletilmiş aralığın ilk kodu
    RwInt32    extCount;      ///< +0x128 genişletilmiş aralık uzunluğu
    RwUInt16  *extTable;      ///< +0x12C (kod - extMin) → glif indeksi
    RwInt32    glyphCount;    ///< +0x130 ayrıştırma sırasında kapasite,
                              ///<   bitişte etkin glif sayısı (sub_4CDBC0)
    CCFontGlyph *glyphs;      ///< +0x134 glif dizisi
    void      *drawHandler;   ///< +0x138 M1: sub_4CCC40 (doku), M3:
                              ///<   sub_4CCAF0 (vektör kontur). Portta tek
                              ///<   yolla FontSystem_DrawText (Aşama 2/3).
    RwUInt32   reserved1;     ///< +0x13C (sub_4CD4D0: 0)
    RwUInt32   reserved2;     ///< +0x140 (sub_4CD4D0: 0)
};

/**
 * @brief Metin çizim materyali — binary sub_4CCC40/sub_4D54C0 arg_10.
 *
 * Düz renk yolu (+0x64 bayrağı yok): baytlar +0x60..+0x63 RGBA.
 * Aydınlatma yolu (+0x64 & 1): her tepe noktası ambient + yönlü ışık ×
 * yatay degrade (invokeWideText'te invW = 1/genişlik; vektör raster'de
 * grad = yol uzunluğu/toplam) ile renklendirilir.
 * Dokulu stroke (+0x64 & 2): UV degrade — u = uStart + uDelta×grad
 * (tepe +0x10/+0x28, taban +0x40/+0x58; sub_4D54C0).
 * strokeWidth (+0x70): vektör kontur yarı genişliği (sub_4D54C0 var_84).
 * Yerleşim binary saha aralıklarıyla birebir şemalanmıştır.
 */
struct CCFontMaterial {
    RwReal topAmbient[4];    ///< +0x00 (tepe vertexleri ambient RGBA)
    RwReal uTop;             ///< +0x10 dokulu stroke: tepe U (grad 0)
    RwReal vTop;             ///< +0x14
    RwReal topLight[4];      ///< +0x18 (tepe vertexleri degrade yön/renk)
    RwReal uTopDelta;        ///< +0x28 dokulu stroke: tepe U delta
    RwReal vTopDelta;        ///< +0x2C
    RwReal bottomAmbient[4]; ///< +0x30 (taban vertexleri ambient RGBA)
    RwReal uBottom;          ///< +0x40 dokulu stroke: taban U (grad 0)
    RwReal vBottom;          ///< +0x44
    RwReal bottomLight[4];   ///< +0x48 (taban vertexleri degrade)
    RwReal uBottomDelta;     ///< +0x58 dokulu stroke: taban U delta
    RwReal vBottomDelta;     ///< +0x5C
    RwUInt8  red;            ///< +0x60 düz renk
    RwUInt8  green;          ///< +0x61
    RwUInt8  blue;           ///< +0x62
    RwUInt8  alpha;          ///< +0x63
    RwUInt8  flags;          ///< +0x64 bit0 = aydınlatma, bit1 = dokulu stroke
    RwUInt8  pad04[3];       ///< +0x65
    RwTexture *texture;      ///< +0x68 (binary doku durumu — glif
                              ///<   dokuları glyph'ten gelir)
    RwReal pad05;            ///< +0x6C (32-bit binary düzeni; 64-bit portta
                             ///<   işaretçi genişliği kayar)
    RwReal strokeWidth;      ///< +0x70 vektör stroke yarı genişliği (var_84)
};

/**
 * @brief Ad önbelleği düğümü — binary 12 bayt (havuz dword_66D5D4).
 *
 * sub_4CEE90: font adı istendiğinde önce bu bağlı listede aranır; bulunamazsa
 * FontSystemLoad ile yüklenir ve başa eklenir.
 */
struct CCFontNode {
    char        *pName;  ///< +0x00 strdup'lanmış istenen ad
    CCFont      *pFont;  ///< +0x04 yüklenen font (başarısızsa düğüm kurulmaz)
    CCFontNode  *pNext;  ///< +0x08 sonraki düğüm (baş = dword_66D5D8)
};

/**
 * @brief Doku okuyucu — sub_5154D0 (ad, maske → doku) karşılığı.
 *
 * Varsayılan RwTextureRead rotası gerçek doku yokluğunda NULL döndürür ve
 * font yüklemesi binary'deki gibi başarısız olur; birim testler sahte
 * okuyucu enjekte eder (boyutlar UV hesabı için gerekli: binary doku
 * raster genişlik/yüksekliğini okur).
 */
typedef RwTexture *(*FontTextureReader)(const char *pName, const char *pMask,
                                        RwInt32 *pWidth, RwInt32 *pHeight);

/**
 * @brief Binary font akışı — sub_50D940/sub_50CD80/sub_50D790 karşılığı.
 *
 * sub_4CC690 (stream font yükleyici) akış soyutlaması: çağıran bellek
 * dosya/asset stream'ini bu üç geri çağırı ile bağlar. pUser çağıranın
 * durumudur (imleç, dosya vbg.).
 */
struct CCFontStream {
    void *pUser;
    /// sub_50D940 — size bayt oku; tamamı okunduysa TRUE (EOF → FALSE).
    RwBool (*read)(CCFontStream *pSelf, void *pBuf, RwInt32 size);
    /// sub_50CD80 — tam okuma; dönen bayt sayısı size ise TRUE.
    RwBool (*readExact)(CCFontStream *pSelf, void *pBuf, RwInt32 size);
    /// sub_50D790 — chunk ara/enter (type-5 kontur öncesi 0x1A1).
    RwBool (*findChunk)(CCFontStream *pSelf, RwUInt32 type);
};

/**
 * @brief Binary stream font yükleyici (sub_4CC690 portu).
 *
 * Akış: tip etiketi 4B ((etiket>>24) >= 1), font başlığı (refCount,
 * lineExtent, sizeScale, spacing, glyphFlags, extMin, glyphCount,
 * extCount), extTable (extCount×2), ascii harita (0x100), glifler.
 * Font başlığındaki refCount alanı YOL SEÇER: 0 → doku yolu (her glif
 * UV+aspect+flags baytı, doku adları 0x20; glyph.texture = textures[
 * (RwInt8)flags]);≠0 → vektör yolu (her glif: atık 4B + aspect + tip;
 * tip==5 → findChunk(0x1A1) + kontur kaydı). drawHandler portta 0
 * (FontSystem_DrawText tek yol).
 *
 * SAPMALAR (belgeli): doku bağlamı kaydet/geri al (sub_5154C0/5154A0)
 * no-op; hazır stroke kayıtları (0x18) CCFontContour yol noktalarına
 * çevrilir (type0 → moveto, diğer → lineto; normal/uzunluk render'da
 * yeniden hesaplanır — miter yok, 3b3b sapması Aşama 3b ile aynı);
 * kontur[+0x10]/node+8 bayrak alanları okunur ama yok sayılır; doku
 * indeksi [0,3] aralığı portta doğrulanır (binary doğrulamaz).
 *
 * @param pStream Akış (geri çağırılar dolu; NULL → NULL).
 * @return Yüklenen font; başarısızsa NULL (font serbest bırakılır).
 * @see FUN_004cc690 (sub_491700 zinciri)
 */
CCFont *FontSystem_LoadFromStream(CCFontStream *pStream);

/**
 * @brief Font vertex tamponu + çizim havuzunu oluşturur.
 * @return Tahsis başarılıysa TRUE.
 * @see FUN_004cefd0 (sub_4CC140 zinciri, 3. adım)
 */
RwBool FontSystemAllocBuffers(void);

/**
 * @brief Font alt sistemini başlatır: "fonts/" yol tamponu, 64 çeyrek-
 * dörtgenlik indeks tablosu ve liste başları.
 * @return Başarıda TRUE.
 * @see FUN_004cd660 (sub_4CC140 zinciri, 4. adım)
 */
RwBool FontSystemInit(void);

/**
 * @brief "fonts/<ad>.met" dosyasını yükler (METRICS1).
 *
 * @param pFontList Noktalı virgülle ayrılmış yedek ad listesi
 *                  (binary: kopya döngüsü + strcat + ';').
 * @return Başarılı ilk adayın fontu; hiçbiri olmazsa NULL.
 * @see FUN_004cd9a0 (dword_66DCC0 yükleme geri çağrısı)
 */
CCFont *FontSystemLoad(const char *pFontList);

/**
 * @brief Ad ile font arar; yoksa yükler ve ad önbelleğine ekler.
 * @param pFontName Aranan font adı.
 * @return Önbellek düğümü (pFont alanı); başarısızsa NULL.
 * @see FUN_004cee90
 */
CCFontNode *FontSystemGet(const char *pFontName);

/**
 * @brief Glif metriğini kodla arar (ASCII doğrudan + genişletilmiş harita).
 * @param pFont Font nesnesi (NULL olabilir).
 * @param code Unicode kod noktası (0..127 doğrudan; >=0x80 extTable).
 * @return Glif kaydı; bulunamazsa NULL.
 */
const CCFontGlyph *FontSystemGetGlyph(const CCFont *pFont, RwUInt32 code);

/**
 * @brief Font önbelleklerini ve havuzlarını serbest bırakır
 *        (yol tamponu, ad listesi, fontlar, genişletilmiş harita).
 * @see FUN_004cd580 (__ioterm+62)
 */
void FontSystemTeardown(void);

/**
 * @brief Vertex tamponunu ve 0x78 çizim havuzunu serbest bırakır.
 * @see FUN_004cef90 (__ioterm+67)
 */
void FontSystemShutdown(void);

/**
 * @brief Doku okuyucu değiştirir (test enjeksiyonu / sonraki fazlar).
 * @param pReader NULL = varsayılan RwTextureRead rotasına dön.
 */
void FontSystem_SetTextureReader(FontTextureReader pReader);

/**
 * @brief 64 çeyrek-dörtgenlik indeks tablosuna erişim (sub_4CD660 üretir).
 * @return [64][6] RwUInt16 tablosu (binary: unk_66D2C4..0x66D5C4).
 */
const RwUInt16 (*FontSystem_GetQuadIndices(void))[6];

/**
 * @brief Metni glif ilerlemeleriyle ölçer (sub_4CD7E0 portu).
 *
 * Döngü: her ünite → glif arama; bulunamayan glif ATLANIR (ilerleme
 * eklenmez); bulunanlarda ölçek (aspect + pFont->spacing) toplanır ve
 * scale ile çarpılır.
 *
 * Geniş mod: font->glyphFlags&2 ise metin UTF-16LE'dir — ünite 2 bayt
 * (düşük önce), imleç adım 2; binary imleç signed <=0 ile durur (kodlar
 * 0x8000+ ise erken çıkış — binary davranışıyla aynı).
 *
 * @param pFont Font nesnesi (NULL → 0).
 * @param pText NUL-sonlu dize (8-bit veya UTF-16LE — glyphFlags&2).
 * @param scale Ölçek çarpanı (binary arg_8 — piksel yüksekliği).
 * @return Toplam genişlik (piksel); pFont NULL ise 0.
 * @see FUN_004CD7E0
 */
RwReal FontSystem_MeasureText(const CCFont *pFont, const char *pText,
                              RwReal scale);

/**
 * @brief Metni Im2D dörtgenleri olarak çizer (sub_4CCC40 portu).
 *
 * Binary akışı (çekirdek — matris yığını/engine vcall'leri hariç,
 * bkz. aşağıdaki sapma notları):
 *  1. Karakter → glif (FontSystemGetGlyph); bulunamayan atlanır.
 *  2. Doku değişimi veya 63. çeyrek-dörtgende batch flush
 *     (RwIm2DRenderIndexedPrimitive, rwPRIMTYPETRILIST; indeksler
 *     sub_4CD660 tablosunun karşılığı: {v0,v2,v3,v0,v3,v1}).
 *  3. Çeyrek-dörtgen: V0(pen,0) V1(pen,1) V2(pen+aspect,0) V3(pen+aspect,1);
 *     UV'ler binary sırasıyla (u0,v1)(u0,v0)(u1,v1)(u1,v0).
 *  4. İlerleme: pen += aspect + pFont->spacing.
 *  5. Renk: materyal düz baytları (+0x60..63) veya aydınlatma yolu
 *     (+0x64&1) degrade ambient + ışık × (pen/genişlik).
 *  6. Dönüş: *pPosX += pen * scale (binary son fadd).
 *
 * Sapma notları (Faz 4.4b'ye kadar):
 *  - Binary vertex'leri birim uzayda (y ∈ {0,1}) üretip matris yığınıyla
 *    (sub_4CC2F0/4CC4D0/4CC480, engine matris stack'i) ölçekler/çevirir;
 *    port ekran koordinatlarını doğrudan üretir (x*scale, y*scale).
 *  - z/rhw: binary dword_66D7A0 (kamera) — port 0.0f/1.0f.
 *  - Geniş (UTF-16LE) metin modu (glyphFlags&2) desteklenir — MeasureText
 *    ile aynı ünite çözümlemesi.
 *  - Doku'suz (METRICS3 vektör) gliflerde quad üretilmez; kontur raster
 *    sub_4D54C0 (Faz 4.4b TODO) — ilerleme yine uygulanır.
 *
 * @param pFont     Font nesnesi.
 * @param pText     NUL-sonlu dize (8-bit veya UTF-16LE — glyphFlags&2).
 * @param scale     Ölçek (piksel yüksekliği; binary arg_8).
 * @param pPosX     Giriş: sol X; çıkış: pen*x_scale sonrası (inout).
 * @param pPosY     Sol Y (vertex y=0 üst, y=1 alt — binary birim uzay).
 * @param pMaterial Materyal (NULL → düz beyaz).
 * @return İlerleme (pen*scale) — sonraki satır/konum için zincirleme.
 * @see FUN_004CCC40 (font->drawHandler)
 */
RwReal FontSystem_DrawText(const CCFont *pFont, const char *pText,
                           RwReal scale, RwReal *pPosX, RwReal *pPosY,
                           const CCFontMaterial *pMaterial);

/**
 * @brief Son batch'in vertex dizisine test erişimi (sub_4CCC40 portu).
 * @param pVertexCount Çıkış: son batch vertex sayısı (quad × 4); NULL olabilir.
 * @return Vertex tamponu (AllocBuffers öncesi NULL); birden fazla batch'te
 *         yalnız son batch geçerli (vertex'ler tamponda yeniden yazılır).
 */
const RwIm2DVertex *FontSystem_GetDrawVertices(RwInt32 *pVertexCount);

#endif // FONTSYSTEM_H

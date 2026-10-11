#ifndef PERFORMANCEVISUALIZER_H
#define PERFORMANCEVISUALIZER_H

// ============================================================================
// CannonCruise - Performance Visualizer (PerformanceVisualizer.h)
// Original path: D:\Projects\CannonCruisePC\code\PerformanceVisualizer.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00498020 - FUN_004981e0 (CPerformanceVisualizer)
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

/**
 * @class CPerformanceVisualizer
 * @brief Ekran üzerinde gerçek zamanlı performans bilgisi görüntüleyen hata ayıklama aracı.
 *
 * CRealtimeProfiler ve CProfilerBehaviour ile entegre çalışarak FPS,
 * kare süresi ve sistem yükü gibi metrikleri Im2D (RenderWare 2D primitif)
 * API'si aracılığıyla ekrana çizer. Yalnızca geliştirme/debug yapılarında etkindir.
 *
 * **Render davranışı:**
 * - `m_bEnabled` FALSE ise `Render()` erken döner ve hiçbir şey çizmez.
 * - Metin çizimi için `m_pCharset` (RtCharset) kullanılır.
 * - Profil çubukları Im2D primitifleriyle çizilir.
 *
 * **Binary referansları:**
 * - Ctor: `FUN_00498020`
 * - Render: `FUN_004980a0` (stub — gövde henüz doldurulmamış)
 * - HandleEvents: `FUN_00498180`
 */
class CPerformanceVisualizer : public RWS::CEventHandler
{
public:
    /**
     * @brief Varsayılan yapıcı — charset kaynağını başlatır, olaylara kaydolur.
     */
    CPerformanceVisualizer();

    /**
     * @brief Yıkıcı — charset kaynağını serbest bırakır, olay kayıtlarını iptal eder.
     */
    virtual ~CPerformanceVisualizer();

    /**
     * @brief Gelen performans profil olaylarını işler.
     * @param msg İşlenecek CMsg olayı.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Performans metriklerini ekrana çizer.
     *
     * `m_bEnabled` FALSE ise hiçbir şey çizmez.
     * Im2D primitifleri ve RtCharset kullanılarak FPS ve profil çubukları
     * ekranın üst bölümüne yerleştirilir.
     *
     * @note Binary adresi `FUN_004980a0` — gövde henüz tam doldurulmamış (stub).
     */
    void Render();

    /**
     * @brief Görselleştiriciyi etkinleştirir veya devre dışı bırakır.
     * @param bEnable TRUE ise sonraki Render() çağrıları aktif olur.
     */
    void SetEnabled(RwBool bEnable) { m_bEnabled = bEnable; }

private:
    RwBool m_bEnabled;   ///< Görselleştirici etkin mi?
    void* m_pCharset;    ///< Metin çizimi için RtCharset işaretçisi
};

#endif // PERFORMANCEVISUALIZER_H

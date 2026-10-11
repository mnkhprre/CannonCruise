#ifndef RWS_ATTACHPLUGINS_H
#define RWS_ATTACHPLUGINS_H

// ============================================================================
// CannonCruise - RenderWare Plugin Attachment Header (attachplugins.h)
// Orijinal dosya: D:\Projects\CannonCruisePC\code\framework\startup\attachplugins.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>

/**
 * @brief Tüm RenderWare eklentilerini başlatır ve motora bağlar.
 *
 * `RwEngineInit()` çağrısından ÖNCE çalıştırılmalıdır. HAnim, Skin, 
 * Collision ve diğer platform spesifik (örn. D3D9) plugin'lerin
 * sisteme kaydedilmesini sağlar.
 *
 * **Binary ref:** `FUN_00419e40`
 *
 * @return TRUE ise eklentiler başarıyla bağlandı. Başarısızlık durumunda 
 *         uygulama genellikle assert ile durur.
 */
RwBool AttachPlugins(void);

#endif // RWS_ATTACHPLUGINS_H


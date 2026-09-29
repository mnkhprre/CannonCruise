// ============================================================================
// CannonCruise - RenderWare Plugin Attachment Header (attachplugins.h)
// Orijinal dosya: D:\Projects\CannonCruisePC\code\framework\startup\attachplugins.h
// ============================================================================
#pragma once

#include <rwcore.h>

// AttachPlugins (FUN_00419e40)
// Tüm RenderWare eklentilerini başlatır ve motora bağlar.
// RwEngineInit() çağrısından ÖNCE çalıştırılmalıdır.
// Başarılıysa TRUE döner, başarısızsa assert ile durur.
RwBool AttachPlugins(void);

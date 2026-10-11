#pragma once

#ifndef _RWREG_H_
#define _RWREG_H_

#include <string>

// ============================================================================
// CannonCruise - RenderWare Registry Interface (rwreg.h)
// ============================================================================

/**
 * @file rwreg.h
 * @brief Windows Kayıt Defteri (Registry) erişimi için sarmalayıcı fonksiyonlar.
 *
 * RenderWare motorunun ve uygulamanın ayarlarını Windows Registry üzerinden
 * okumak/yazmak için kullanılır. (Örn: Çözünürlük ayarları, ses seviyeleri, vb.)
 */

/**
 * @brief Belirtilen yola string türünde bir kayıt defteri değeri yazar.
 * @param path   Ana yol (Örn: "Software\\RenderWare")
 * @param subKey Alt anahtar adı
 * @param entry  Değerin adı
 * @param value  Yazılacak string değer
 */
void RwRegSetString(const std::string& path, const std::string& subKey, const std::string& entry, const std::string& value);

/**
 * @brief Belirtilen yoldan string türünde bir kayıt defteri değeri okur.
 * @param path         Ana yol
 * @param subKey       Alt anahtar adı
 * @param entry        Değerin adı
 * @param defaultValue Değer bulunamazsa döndürülecek varsayılan string
 * @return std::string Okunan değer veya varsayılan değer
 */
std::string RwRegGetString(const std::string& path, const std::string& subKey, const std::string& entry, const std::string& defaultValue);

/**
 * @brief Belirtilen yola tamsayı (int) türünde bir kayıt defteri değeri yazar.
 * @param path   Ana yol
 * @param subKey Alt anahtar adı
 * @param entry  Değerin adı
 * @param value  Yazılacak tamsayı değer
 */
void RwRegSetInt(const std::string& path, const std::string& subKey, const std::string& entry, int value);

/**
 * @brief Belirtilen yoldan tamsayı (int) türünde bir kayıt defteri değeri okur.
 * @param path         Ana yol
 * @param subKey       Alt anahtar adı
 * @param entry        Değerin adı
 * @param defaultValue Değer bulunamazsa döndürülecek varsayılan tamsayı
 * @return int         Okunan değer veya varsayılan değer
 */
int RwRegGetInt(const std::string& path, const std::string& subKey, const std::string& entry, int defaultValue);

#endif // _RWREG_H_


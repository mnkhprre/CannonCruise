#include "rwreg.h"

#define WIN32_LEAN_AND_MEAN
#include <cassert>
#include <windows.h>

// ============================================================================
// CannonCruise - RenderWare Registry Interface (rwreg.cpp)
// Orijinal dosya:
// D:\Projects\CannonCruisePC\code\framework\startup\win32\rwreg.cpp Tersine
// mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Tüm RenderWare ayarları Windows Registry'de şu kök altında tutulur:
//   HKEY_CURRENT_USER\Software\Criterion\RenderWare\<path>
// ============================================================================

// RenderWare'in kayıt defteri kök yolu (0x00612e88)
static const char *const RW_REG_ROOT = "Software\\Criterion\\RenderWare\\";

// Ortak: tam anahtar yolunu oluştur
// Örn: path="CannonCruise/3.6", subKey="" ->
// "Software\Criterion\RenderWare\CannonCruise/3.6"
static std::string BuildKeyPath(const std::string &path,
                                const std::string &subKey) {
  std::string fullPath = RW_REG_ROOT;
  if (!path.empty()) {
    fullPath += path;
    if (!subKey.empty() && fullPath.back() != '\\') {
      fullPath += '\\';
    }
  }
  if (!subKey.empty()) {
    fullPath += subKey;
  }
  return fullPath;
}

// ============================================================================
// RwRegSetString (FUN_00416080)
// Binary Adresi: 0x00416080
// Görevi: Belirtilen anahtar ve değer adı altında bir string yazar.
//   - Adresler: RegCreateKeyExA (0x002103FEh), RegSetValueExA (0x002103ECh),
//   RegCloseKey
//   - Assert: !entry.empty() (0x00612eec)
//   - Kök: HKEY_CURRENT_USER
// ============================================================================
void RwRegSetString(const std::string &path, const std::string &subKey,
                    const std::string &entry, const std::string &value) {
  assert(!entry.empty() && "!entry.empty() Failed");

  std::string fullKey = BuildKeyPath(path, subKey);

  HKEY hKey = NULL;
  DWORD dwDisposition = 0;
  LONG result = RegCreateKeyExA(
      HKEY_CURRENT_USER, // hKey for RegCreateKeyExA (DAT_80000001)
      fullKey.c_str(),   // lpSubKey
      0,                 // Reserved
      NULL,              // lpClass (lpClass_00635580 -> empty string)
      0,                 // dwOptions
      KEY_ALL_ACCESS,    // samDesired (0x1F = KEY_ALL_ACCESS — 0x0000001F)
      NULL,              // lpSecurityAttributes
      &hKey,             // phkResult
      &dwDisposition     // lpdwDisposition
  );

  if (result == ERROR_SUCCESS) {
    // +1 for null terminator — matches INC EDX at 0x0041632c
    DWORD cbData = static_cast<DWORD>(value.size()) + 1;
    RegSetValueExA(hKey,          // HKEY
                   entry.c_str(), // lpValueName
                   0,             // Reserved
                   REG_SZ,        // dwType = 1 (0x00416333)
                   reinterpret_cast<const BYTE *>(value.c_str()), // lpData
                   cbData                                         // cbData
    );
    RegCloseKey(hKey);
  }
}

// ============================================================================
// RwRegGetString (FUN_00416690)
// Binary Adresi: 0x00416690
// Görevi: Belirtilen anahtar altındaki string değerini okur.
//   - İlk HKCU dener (okuma modu: 0x20019), başarısız olursa HKLM dener,
//     o da başarısız olursa HKCU'da yeni anahtar oluşturur ve varsayılanı
//     yazar.
//   - Assert: !entry.empty()
// ============================================================================
std::string RwRegGetString(const std::string &path, const std::string &subKey,
                           const std::string &entry,
                           const std::string &defaultValue) {
  assert(!entry.empty() && "!entry.empty() Failed");

  std::string fullKey = BuildKeyPath(path, subKey);

  // Önce HKCU'yu oku (READ_ONLY)
  HKEY hKey = NULL;
  LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, // DAT_80000001
                              fullKey.c_str(), 0,
                              KEY_READ | KEY_WRITE, // samDesired = 0x20019
                              &hKey);

  if (result != ERROR_SUCCESS) {
    // HKCU başarısızsa HKLM dene (READ_ONLY)
    result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, // DAT_80000002
                           fullKey.c_str(), 0,
                           KEY_READ | KEY_WRITE, // samDesired = 0x20019
                           &hKey);

    if (result != ERROR_SUCCESS) {
      // İkisi de başarısız: HKCU'da yeni anahtar oluştur,
      // varsayılan değeri yaz (FUN_00416080 call at 0x004169a7)
      RwRegSetString(path, subKey, entry, defaultValue);
      return defaultValue;
    }
  }

  // Anahtar açıldı, değeri sorgula
  char buffer[MAX_PATH] = {};
  DWORD dwType = REG_SZ;
  DWORD dwSize = sizeof(buffer);
  result = RegQueryValueExA(hKey, entry.c_str(), NULL, &dwType,
                            reinterpret_cast<LPBYTE>(buffer), &dwSize);
  RegCloseKey(hKey);

  if (result == ERROR_SUCCESS && dwType == REG_SZ) {
    return std::string(buffer);
  }

  return defaultValue;
}

// ============================================================================
// RwRegSetInt (FUN_00416080 ile aynı mekanizma, REG_DWORD)
// Tamsayı yardımcıları — binary'de ayrı bir fonksiyon yoktur;
// tamsayılar da REG_SZ (string) olarak depolanır, sscanf ile okunur.
// ============================================================================
void RwRegSetInt(const std::string &path, const std::string &subKey,
                 const std::string &entry, int value) {
  assert(!entry.empty() && "!entry.empty() Failed");
  RwRegSetString(path, subKey, entry, std::to_string(value));
}

int RwRegGetInt(const std::string &path, const std::string &subKey,
                const std::string &entry, int defaultValue) {
  assert(!entry.empty() && "!entry.empty() Failed");
  std::string s =
      RwRegGetString(path, subKey, entry, std::to_string(defaultValue));
  try {
    return std::stoi(s);
  } catch (...) {
    return defaultValue;
  }
}

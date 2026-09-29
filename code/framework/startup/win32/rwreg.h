#pragma once

#ifndef _RWREG_H_
#define _RWREG_H_

#include <string>

// ============================================================================
// CannonCruise - RenderWare Registry Interface (rwreg.h)
// ============================================================================

// String Tabanlı Kayıt Defteri Okuma / Yazma
void RwRegSetString(const std::string& path, const std::string& subKey, const std::string& entry, const std::string& value);
std::string RwRegGetString(const std::string& path, const std::string& subKey, const std::string& entry, const std::string& defaultValue);

// Tamsayı (Int) Tabanlı Kayıt Defteri Okuma / Yazma
void RwRegSetInt(const std::string& path, const std::string& subKey, const std::string& entry, int value);
int RwRegGetInt(const std::string& path, const std::string& subKey, const std::string& entry, int defaultValue);

#endif // _RWREG_H_

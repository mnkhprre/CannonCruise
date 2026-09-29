#ifndef HAVOKMEMORYMANAGER_H
#define HAVOKMEMORYMANAGER_H

// ============================================================================
// CannonCruise - Havok Physics Memory Manager (HavokMemoryManager.h)
// Original path: D:\Projects\CannonCruisePC\code\HavokMemoryManager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>

class CHavokMemoryManager {
public:
  static void *Allocate(RwUInt32 size, RwUInt32 alignment = 16);
  static void Free(void *pMem);
  static void Initialize();
  static void Shutdown();
};

#endif // HAVOKMEMORYMANAGER_H

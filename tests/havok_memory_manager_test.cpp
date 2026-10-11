// SDK-independent regression test for the local allocation helper.
// This does not initialize or validate Havok.
#include "HavokMemoryManager.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <initializer_list>

#define CHECK(condition) do { if (!(condition)) { \
    std::fprintf(stderr, "FAIL at line %d: %s\n", __LINE__, #condition); \
    return 1; } } while (0)

int main()
{
    const RwUInt32 sizes[] = {0, 1, 15, 16, 17, 4096, 1048576};
    for (RwUInt32 size : sizes)
    {
        void* memory = CHavokMemoryManager::Allocate(size);
        CHECK(memory != nullptr);
        CHECK(reinterpret_cast<std::uintptr_t>(memory) % 16 == 0);
        if (size) std::memset(memory, 0x5a, size);
        CHavokMemoryManager::Free(memory);
    }
    CHavokMemoryManager::Free(nullptr);
#ifdef NDEBUG
    // The public helper currently supports only alignment=16.
    for (RwUInt32 alignment : {0u, 1u, 3u, 8u, 32u, 128u})
        CHECK(CHavokMemoryManager::Allocate(32, alignment) == nullptr);
#endif
    if (sizeof(size_t) == sizeof(RwUInt32))
        CHECK(CHavokMemoryManager::Allocate(std::numeric_limits<RwUInt32>::max()) == nullptr);
    std::puts("PASS: local allocator alignment, write/free, null-free and applicable guard cases");
    return 0;
}

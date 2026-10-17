#ifndef HK_BASE_TYPES_H
#define HK_BASE_TYPES_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Shim (hkBaseTypes.h)
// Binary: E:\nightly_builds\Havok_2_2_1_Branch\build\code\hk221\source\hkbase
// Reverse-engineered interfaces matching CC.asm / CC_ida.asm types.
// SHIM: havokimpl.cpp binds them to Bullet engine (verified by havokshim_test).
// ============================================================================

#include <cstddef>

typedef int          hkBool;
typedef signed char  hkChar;
typedef signed char  hkInt8;
typedef unsigned char hkUint8;
typedef short        hkInt16;
typedef unsigned short hkUint16;
typedef int          hkInt32;
typedef unsigned int hkUint32;
typedef long long    hkInt64;
typedef unsigned long long hkUint64;
typedef float        hkReal;
typedef double       hkDouble64;
typedef void*        hkPointer;

#ifndef HK_NULL
#define HK_NULL 0
#endif

#ifndef HK_INLINE
#ifdef _WIN32
#define HK_INLINE __forceinline
#else
#define HK_INLINE inline
#endif
#endif

#ifndef HK_CALL
#define HK_CALL
#endif

#ifndef HK_ON_ASSERT
#define HK_ON_ASSERT(X) X
#endif

// Havok 2.x error code — "hkResult" appears in ASM
struct hkResult
{
    enum { FAIL = 0, SUCCESS = 1 } m_enum;
    hkResult(hkBool b = false) : m_enum(b ? SUCCESS : FAIL) {}
    hkBool isFailure() const { return m_enum == FAIL; }
    hkBool isSuccess() const { return m_enum == SUCCESS; }
};

// Simple 3-vector (3D counterpart of hkVector4; game works with RwV3d)
struct hkVector3
{
    hkReal x, y, z;
    hkVector3() : x(0.0f), y(0.0f), z(0.0f) {}
    hkVector3(hkReal xx, hkReal yy, hkReal zz) : x(xx), y(yy), z(zz) {}
};

// AABB bounding box (for hkBvTreeShape broad-phase)
struct hkAabb
{
    hkVector3 m_min;
    hkVector3 m_max;
};

#endif // HK_BASE_TYPES_H

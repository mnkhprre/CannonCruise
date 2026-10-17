#ifndef HK_CLASS_H
#define HK_CLASS_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Shim (hkClass.h)
// Binary tipler: hkClass, hkSerializable, hkType* (hkTypeInt, hkTypeReal, ...)
// Havok 2.x serialization / reflection infrastructure.
// ============================================================================

#include "hkBaseTypes.h"

// Serialization field types (ASM: hkTypeInt, hkTypeReal, hkTypeVector, ...)
enum hkDataType
{
    HK_TYPE_VOID,
    HK_TYPE_BOOL,
    HK_TYPE_CHAR,
    HK_TYPE_INT,
    HK_TYPE_UINT,
    HK_TYPE_REAL,
    HK_TYPE_VECTOR,
    HK_TYPE_MATRIX,
    HK_TYPE_QUATERNION,
    HK_TYPE_ROTATION,
    HK_TYPE_TRANSFORM,
    HK_TYPE_POINTER,
    HK_TYPE_ARRAY,
    HK_TYPE_MAX
};

struct hkClassField
{
    const char* m_name;
    hkUint32    m_offset;
    hkUint32    m_size;
    hkDataType  m_type;
    const hkClassField* m_classField; ///< For nested class fields
};

/// Havok class reflection — "hkClass" appears in ASM
class hkClass
{
public:
    hkClass(const char* name, const hkClass* parent, const hkClassField* fields,
            int numFields, int totalSize)
        : m_name(name), m_parent(parent), m_fields(fields),
          m_numFields(numFields), m_totalSize(totalSize) {}
    virtual ~hkClass() {}

    const char* getName() const { return m_name; }
    const hkClass* getParent() const { return m_parent; }
    const hkClassField* getFields() const { return m_fields; }
    int getNumFields() const { return m_numFields; }
    int getTotalSize() const { return m_totalSize; }

private:
    const char* m_name;
    const hkClass* m_parent;
    const hkClassField* m_fields;
    int m_numFields;
    int m_totalSize;
};

/// Base of all serializable Havok objects
class hkSerializable
{
public:
    virtual ~hkSerializable() {}

    /// Class reflection
    virtual const hkClass* getClass() const = 0;

    /// Write to stream — binary stream serialization not yet wired (Phase 4.6).
    virtual hkResult writeToStream(void* stream) const
    {
        (void)stream;
        return hkResult(hkResult::FAIL);
    }

    /// Read from stream — see writeToStream.
    virtual hkResult readFromStream(void* stream)
    {
        (void)stream;
        return hkResult(hkResult::FAIL);
    }
};

#endif // HK_CLASS_H

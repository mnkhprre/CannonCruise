#ifndef RWS_CSYSTEMCOMMANDS_H
#define RWS_CSYSTEMCOMMANDS_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (csystemcommands.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\attributehandler\csystemcommands.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>

namespace RWS
{
    enum SystemCommandId
    {
        CMD_LoadMatrix = 0x01,
        CMD_SetParent  = 0x02,
        CMD_SetFlags   = 0x03,
        CMD_AttachAtomic = 0x04
    };

    struct CAttributeCommand
    {
        RwUInt32 m_CommandId;
        RwUInt32 m_DataSize;
        const void* m_pData;

        RwUInt32 GetCommandId() const { return m_CommandId; }
        const void* GetData() const { return m_pData; }
        RwUInt32 GetDataSize() const { return m_DataSize; }
    };

    class CSystemCommands
    {
    public:
        // Komut işleyicileri
        static void HandleLoadMatrix(RwFrame* pFrame, const CAttributeCommand& attrCmd);
        static void HandleSetParent(RwFrame* pChildFrame, RwFrame* pParentFrame);
        static void HandleAttachAtomic(RpAtomic* pAtomic, RwFrame* pFrame);
        static void HandleSetFlags(RpAtomic* pAtomic, RwUInt32 flags);
    };
}

#endif // RWS_CSYSTEMCOMMANDS_H

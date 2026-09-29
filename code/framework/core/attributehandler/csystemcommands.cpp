// ============================================================================
// CannonCruise - RenderWare Studio Framework (csystemcommands.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\attributehandler\csystemcommands.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00415620 — CSystemCommands::HandleLoadMatrix
//   FUN_00415ac0 — CSystemCommands::HandleSetParent
// ============================================================================

#include "csystemcommands.h"
#include <cassert>
#include <cstring>

namespace RWS
{
    // ========================================================================
    // CSystemCommands::HandleLoadMatrix (FUN_00415620)
    // ASM: 00415620 — Matris verisini alıp frame'e uygula
    // ========================================================================
    void CSystemCommands::HandleLoadMatrix(RwFrame* pFrame, const CAttributeCommand& attrCmd)
    {
        assert(pFrame != 0 && "Failed PRE-condition: pObject");
        assert(CMD_LoadMatrix == attrCmd.GetCommandId() && "Failed PRE-condition: CMD_LoadMatrix == attrCmd.GetCommandId()");

        const RwMatrix* pMatrix = (const RwMatrix*)attrCmd.GetData();
        if (pMatrix)
        {
            RwMatrix matrixCopy;
            memcpy(&matrixCopy, pMatrix, sizeof(RwMatrix));
            
            // Matris optimizasyonu & normalizasyon (FUN_0050f690 / RwMatrixUpdate)
            RwMatrixUpdate(&matrixCopy);

            // Frame dönüşümünü güncelle (FUN_0050e910 / RwFrameTransform)
            RwFrameTransform(pFrame, &matrixCopy, rwCOMBINEREPLACE);
        }
    }

    // ========================================================================
    // CSystemCommands::HandleSetParent (FUN_00415ac0)
    // ASM: 00415ac0 — Frame ebeveyn hiyerarşisini bağla
    // ========================================================================
    void CSystemCommands::HandleSetParent(RwFrame* pChildFrame, RwFrame* pParentFrame)
    {
        assert(pChildFrame != 0 && "Failed PRE-condition: pSrcObject");
        assert(pParentFrame != 0 && "Failed PRE-condition: pDstObject");

        RwFrameAddChild(pParentFrame, pChildFrame);
    }

    // ========================================================================
    // CSystemCommands::HandleAttachAtomic
    // ========================================================================
    void CSystemCommands::HandleAttachAtomic(RpAtomic* pAtomic, RwFrame* pFrame)
    {
        assert(pAtomic != 0 && "Failed PRE-condition: pAtomic");
        assert(pFrame != 0 && "Failed PRE-condition: pObject");

        RpAtomicSetFrame(pAtomic, pFrame);
    }

    // ========================================================================
    // CSystemCommands::HandleSetFlags
    // ========================================================================
    void CSystemCommands::HandleSetFlags(RpAtomic* pAtomic, RwUInt32 flags)
    {
        assert(pAtomic != 0 && "Failed PRE-condition: pAtomic");

        RpAtomicSetFlags(pAtomic, flags);
    }
}

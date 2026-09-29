// ============================================================================
// CannonCruise - GUI Minimap (GUIMap.cpp)
// Original path: D:\Projects\CannonCruisePC\code\GUIMap.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00485830 — CGUIMap constructor & registration
//   FUN_004860b0 — CGUIMap::AddSpot
//   FUN_004867f0 — CGUIMap::Render
// ============================================================================

#include "GUIMap.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>
#include <cstring>

RWS_REGISTER_CLASS(CGUIMap, []() -> void* { return new CGUIMap(); }, sizeof(CGUIMap));

CGUIMap::CGUIMap()
    : m_fPlayerHeading(0.0f)
{
    memset(m_apTextures, 0, sizeof(m_apTextures));
}

CGUIMap::~CGUIMap()
{
    ClearSpots();
}

void CGUIMap::HandleEvents(const RWS::CMsg& msg)
{
    // iMsgRenderDotEvent
}

void CGUIMap::AddSpot(const RwV3d* pPos, EMissionTextureType type)
{
    assert(pPos != 0);
    assert(type < MISSION_TEXTURE_COUNT && "Illegal MISSION_TEXTURE_TYPE");

    SMissionIndicatorData spot;
    spot.m_WorldPos = *pPos;
    spot.m_Type = type;
    spot.m_bActive = TRUE;

    m_Spots.push_back(spot);
}

void CGUIMap::ClearSpots()
{
    m_Spots.clear();
}

void CGUIMap::SetPlayerHeading(RwReal angle)
{
    m_fPlayerHeading = angle;
}

void CGUIMap::Render()
{
    // Pusula ve harita noktalarını 2D olarak çiz
}

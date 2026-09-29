#pragma once
#include <rwcore.h>
#include <rpworld.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"

namespace RWS
{
    /**
     * CSpawnable - Base class for objects that can be spawned/despawned dynamically.
     * Manages active/spawned state with debug asserts for invalid state transitions.
     * Binary ref: FUN_004b9960 - FUN_004ba730, strings at 0x0061920c
     */
    class CSpawnable : public CAttributeHandler, public CEventHandler
    {
    public:
        RWS_MAKENEWCLASS(CSpawnable);
        RWS_DECLARE_CLASS(CSpawnable);

        CSpawnable(const CAttributePacket& packet);
        virtual ~CSpawnable();

        virtual void HandleAttributes(const CAttributePacket& packet);
        virtual void HandleEvents(CMsg& msg);

        void Activate();
        void Deactivate();
        void Spawn();
        void Despawn();

        RwBool IsActive() const { return m_bActive; }
        RwBool IsSpawned() const { return m_bSpawned; }

        virtual RwBool IsAlive() const;

    protected:
        RwBool m_bActive;       // Whether the spawnable is active
        RwBool m_bSpawned;      // Whether the spawnable has been placed into the world
        RwReal m_spawnDelay;    // Delay before spawning
        RwReal m_spawnTimer;    // Current spawn timer
    };
}

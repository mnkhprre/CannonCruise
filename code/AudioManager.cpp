// ============================================================================
// CannonCruise - Audio Manager (AudioManager.cpp)
// ============================================================================

#include "AudioManager.h"
#include "framework/toolkits/Audio/Audio.h"
#include <cassert>
#include <cstring>

// ---------------------------------------------------------------------------
// Constructor – mirrors FUN_00455c00.
// Initializes default volume groups to 1.0, resets listener matrix to identity,
// and configures the sound asset directory ("Sound\\Win32\\").
// ---------------------------------------------------------------------------
CAudioManager::CAudioManager()
    : m_SoundPath("Sound\\Win32\\")
{
    m_afVolumeGroups[0] = 1.0f;
    m_afVolumeGroups[1] = 1.0f;
    m_afVolumeGroups[2] = 1.0f;

    RwMatrixSetIdentity(&m_ListenerMatrix);
}

// ---------------------------------------------------------------------------
// Destructor – halts all active voices before destruction.
// ---------------------------------------------------------------------------
CAudioManager::~CAudioManager()
{
    StopAllVoices();
}

// ---------------------------------------------------------------------------
// HandleEvents – framework event listener for audio commands.
// ---------------------------------------------------------------------------
void CAudioManager::HandleEvents(const RWS::CMsg& msg)
{
}

// ---------------------------------------------------------------------------
// PlaySound – plays a sound cue by resource identifier.
// ---------------------------------------------------------------------------
void CAudioManager::PlaySound(const char* pSoundName, const RwV3d* pPos)
{
    // TODO: Implement WAV playback from Sound\\Win32\\<name>.wav
    // (currently a stub; sound files will be added later).
    (void)pSoundName;
    (void)pPos;
}

// ---------------------------------------------------------------------------
// PlayStream – streams a background music or ambient audio track.
// ---------------------------------------------------------------------------
void CAudioManager::PlayStream(const char* pStreamName, RwBool bLoop)
{
    // TODO: Implement background music streaming.
    // (currently a stub; music files will be added later).
    (void)pStreamName;
    (void)bLoop;
}

// ---------------------------------------------------------------------------
// StopAllVoices – stops all active voice and sound channels.
// ---------------------------------------------------------------------------
void CAudioManager::StopAllVoices()
{
}

// ---------------------------------------------------------------------------
// SetVolumeGroup – sets master, music, or FX volume channel gain.
// ---------------------------------------------------------------------------
void CAudioManager::SetVolumeGroup(RwUInt32 groupIndex, RwReal volume)
{
    if (groupIndex < 3)
    {
        m_afVolumeGroups[groupIndex] = volume;
    }
}

// ---------------------------------------------------------------------------
// GetVolumeGroup – returns volume level of specified group.
// ---------------------------------------------------------------------------
RwReal CAudioManager::GetVolumeGroup(RwUInt32 groupIndex) const
{
    if (groupIndex < 3) {
        return m_afVolumeGroups[groupIndex];
    }
    return 1.0f;
}

// ---------------------------------------------------------------------------
// UpdateListenerPosition – updates 3D listener position.
// ---------------------------------------------------------------------------
void CAudioManager::UpdateListenerPosition(const RwMatrix* pMatrix)
{
    if (pMatrix) {
        m_ListenerMatrix = *pMatrix;
        RWS::Audio::SetListenerPosition(&m_ListenerMatrix.pos, &m_ListenerMatrix.at, &m_ListenerMatrix.up);
    }
}
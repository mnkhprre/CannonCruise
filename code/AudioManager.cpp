// ============================================================================
// CannonCruise - Audio Manager (AudioManager.cpp)
// ============================================================================

#include "AudioManager.h"
#include "framework/toolkits/Audio/Audio.h"
#include <cassert>
#include <cstring>
#include <windows.h>
#include <mmsystem.h>

// Undefine Windows PlaySound macro to avoid conflict with CAudioManager::PlaySound
#ifdef PlaySound
#undef PlaySound
#endif

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
    if (pSoundName == nullptr || pSoundName[0] == 0) {
        return;
    }

    // Build full path: Sound\Win32\<name>.wav
    char fullPath[MAX_PATH] = {0};
    std::snprintf(fullPath, sizeof(fullPath), "%s%s.wav", m_SoundPath.c_str(), pSoundName);

    // Play asynchronously so it doesn't block the game loop
    // SND_FILENAME = sound name is a file path
    // SND_ASYNC = play asynchronously (return immediately)
    // SND_NODEFAULT = don't play default sound if file not found
    DWORD flags = SND_FILENAME | SND_ASYNC | SND_NODEFAULT;
    if (!::PlaySoundA(fullPath, nullptr, flags)) {
        // Silent fail if file not found or playback fails
    }
}

// ---------------------------------------------------------------------------
// PlayStream – streams a background music or ambient audio track.
// ---------------------------------------------------------------------------
void CAudioManager::PlayStream(const char* pStreamName, RwBool bLoop)
{
    if (pStreamName == nullptr || pStreamName[0] == 0) {
        return;
    }

    // Build full path: Sound\Win32\<name>.wav or .rws for music tracks
    char fullPath[MAX_PATH] = {0};
    std::snprintf(fullPath, sizeof(fullPath), "%s%s.wav", m_SoundPath.c_str(), pStreamName);

    // For streaming music, use looping flag if requested
    DWORD flags = SND_FILENAME | SND_ASYNC | SND_NODEFAULT;
    if (bLoop) {
        flags |= SND_LOOP;
    }

    if (!::PlaySoundA(fullPath, nullptr, flags)) {
        // Silent fail if file not found or playback fails
    }
}

// ---------------------------------------------------------------------------
// StopAllVoices – stops all active voice and sound channels.
// ---------------------------------------------------------------------------
void CAudioManager::StopAllVoices()
{
    PlaySoundA(nullptr, nullptr, 0); // Stop all sounds
}

// ---------------------------------------------------------------------------
// SetVolumeGroup – sets master, music, or FX volume channel gain.
// ---------------------------------------------------------------------------
void CAudioManager::SetVolumeGroup(RwUInt32 groupIndex, RwReal volume)
{
    if (groupIndex < 3) {
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

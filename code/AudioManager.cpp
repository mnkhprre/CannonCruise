// ============================================================================
// CannonCruise - Audio Manager (AudioManager.cpp)
// ============================================================================

#include "AudioManager.h"
#include "framework/toolkits/Audio/Audio.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <windows.h>
#include <mmsystem.h>

// Undefine Windows PlaySound macro to avoid conflict with CAudioManager::PlaySound
#ifdef PlaySound
#undef PlaySound
#endif

// ---------------------------------------------------------------------------
// Constructor - mirrors FUN_00455c00.
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
// Destructor - halts all active voices before destruction.
// ---------------------------------------------------------------------------
CAudioManager::~CAudioManager()
{
    StopAllVoices();
}

// ---------------------------------------------------------------------------
// HandleEvents - framework event listener for audio commands.
// ---------------------------------------------------------------------------
void CAudioManager::HandleEvents(const RWS::CMsg& msg)
{
}

// ---------------------------------------------------------------------------
// PlaySound - plays a sound cue by resource identifier.
// Uses PlaySoundA asynchronously so it doesn't interrupt background MCI music.
// ---------------------------------------------------------------------------
void CAudioManager::PlaySound(const char* pSoundName, const RwV3d* pPos)
{
    if (pSoundName == nullptr || pSoundName[0] == 0) {
        return;
    }

    // Build full path: Sound\Win32\<name>.wav
    char fullPath[MAX_PATH] = {0};
    if (std::strstr(pSoundName, ".wav") != nullptr) {
        std::snprintf(fullPath, sizeof(fullPath), "%s%s", m_SoundPath.c_str(), pSoundName);
    } else {
        std::snprintf(fullPath, sizeof(fullPath), "%s%s.wav", m_SoundPath.c_str(), pSoundName);
    }

    // Check if exists directly, otherwise try CWD relative
    if (::GetFileAttributesA(fullPath) == INVALID_FILE_ATTRIBUTES) {
        if (std::strstr(pSoundName, ".wav") != nullptr) {
            std::snprintf(fullPath, sizeof(fullPath), "%s", pSoundName);
        } else {
            std::snprintf(fullPath, sizeof(fullPath), "%s.wav", pSoundName);
        }
    }

    // Play asynchronously so it doesn't block the game loop
    DWORD flags = SND_FILENAME | SND_ASYNC | SND_NODEFAULT;
    ::PlaySoundA(fullPath, nullptr, flags);
}

// ---------------------------------------------------------------------------
// PlayStream - streams a background music or ambient audio track via MCI.
// Runs on a dedicated waveaudio device alias so it mixes cleanly with SFX.
// ---------------------------------------------------------------------------
void CAudioManager::PlayStream(const char* pStreamName, RwBool bLoop)
{
    if (pStreamName == nullptr || pStreamName[0] == 0) {
        return;
    }

    // If already playing the same track, keep playing
    if (m_CurrentStream == pStreamName) {
        return;
    }

    // Stop and close previous background stream
    StopStream();

    // Resolve path: Sound\Win32\<name>.wav or fallback
    char fullPath[MAX_PATH] = {0};
    if (std::strstr(pStreamName, ".wav") != nullptr) {
        std::snprintf(fullPath, sizeof(fullPath), "%s%s", m_SoundPath.c_str(), pStreamName);
    } else {
        std::snprintf(fullPath, sizeof(fullPath), "%s%s.wav", m_SoundPath.c_str(), pStreamName);
    }

    if (::GetFileAttributesA(fullPath) == INVALID_FILE_ATTRIBUTES) {
        // Fallback names
        if (std::strstr(pStreamName, "MenuMusic") != nullptr) {
            std::snprintf(fullPath, sizeof(fullPath), "%sMenuMus.wav", m_SoundPath.c_str());
        } else if (std::strstr(pStreamName, "BackgroundMusic") != nullptr) {
            std::snprintf(fullPath, sizeof(fullPath), "%sBeaverM.wav", m_SoundPath.c_str());
        } else {
            // Try directly in working directory
            std::snprintf(fullPath, sizeof(fullPath), "%s.wav", pStreamName);
        }
    }

    m_CurrentStream = pStreamName;

    char mciCmd[MAX_PATH + 64] = {0};
    std::snprintf(mciCmd, sizeof(mciCmd), "open \"%s\" type waveaudio alias cc_bgm", fullPath);
    MCIERROR err = ::mciSendStringA(mciCmd, nullptr, 0, nullptr);
    if (err == 0) {
        if (bLoop) {
            ::mciSendStringA("play cc_bgm repeat", nullptr, 0, nullptr);
        } else {
            ::mciSendStringA("play cc_bgm", nullptr, 0, nullptr);
        }
    } else {
        // Fallback to PlaySoundA if MCI device unavailable
        DWORD flags = SND_FILENAME | SND_ASYNC | SND_NODEFAULT;
        if (bLoop) {
            flags |= SND_LOOP;
        }
        ::PlaySoundA(fullPath, nullptr, flags);
    }
}

// ---------------------------------------------------------------------------
// StopStream - stops background music.
// ---------------------------------------------------------------------------
void CAudioManager::StopStream()
{
    ::mciSendStringA("stop cc_bgm", nullptr, 0, nullptr);
    ::mciSendStringA("close cc_bgm", nullptr, 0, nullptr);
    m_CurrentStream.clear();
}

// ---------------------------------------------------------------------------
// StopAllVoices - stops all active voice and sound channels.
// ---------------------------------------------------------------------------
void CAudioManager::StopAllVoices()
{
    StopStream();
    ::PlaySoundA(nullptr, nullptr, 0); // Stop all sounds
}

// ---------------------------------------------------------------------------
// SetVolumeGroup - sets master, music, or FX volume channel gain.
// ---------------------------------------------------------------------------
void CAudioManager::SetVolumeGroup(RwUInt32 groupIndex, RwReal volume)
{
    if (groupIndex < 3) {
        m_afVolumeGroups[groupIndex] = volume;
    }
}

// ---------------------------------------------------------------------------
// GetVolumeGroup - returns volume level of specified group.
// ---------------------------------------------------------------------------
RwReal CAudioManager::GetVolumeGroup(RwUInt32 groupIndex) const
{
    if (groupIndex < 3) {
        return m_afVolumeGroups[groupIndex];
    }
    return 1.0f;
}

// ---------------------------------------------------------------------------
// UpdateListenerPosition - updates 3D listener position.
// ---------------------------------------------------------------------------
void CAudioManager::UpdateListenerPosition(const RwMatrix* pMatrix)
{
    if (pMatrix) {
        m_ListenerMatrix = *pMatrix;
        RWS::Audio::SetListenerPosition(&m_ListenerMatrix.pos, &m_ListenerMatrix.at, &m_ListenerMatrix.up);
    }
}

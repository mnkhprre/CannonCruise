#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

// ============================================================================
// CannonCruise - Audio Manager (AudioManager.h)
// Original path: D:\Projects\CannonCruisePC\code\AudioManager.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include "Singleton.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <string>
#include <map>

/**
 * @class CAudioManager
 * @brief Singleton sound engine manager coordinating 2D cues, 3D spatial audio, and streams.
 *
 * Integrates with the RenderWare Audio subsystem (RWS::Audio).
 * Maintains three volume groups (Master, Music/Ambience, Sound FX), manages the
 * 3D listener orientation matrix (`m_ListenerMatrix`), and handles playback requests
 * for sound effects, background stream tracks, and voice lines.
 */
class CAudioManager : public CSingleton<CAudioManager>, public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs audio manager singleton, setting volume groups to 1.0.
     */
    CAudioManager();

    /**
     * @brief Destructor - stops all active sound voices before teardown.
     */
    virtual ~CAudioManager();

    /**
     * @brief Framework message dispatcher (handles listener matrix updates and voice halts).
     * @param msg  Message descriptor.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    // ---- Sound Playback & Control -------------------------------------------

    /**
     * @brief Plays a sound cue by resource identifier with optional 3D world positioning.
     * @param pSoundName  Identifier name of the audio asset (e.g. "Audio_CannonFire").
     * @param pPos        World-space coordinates for spatial panning/attenuation, or NULL for 2D.
     */
    void PlaySound(const char* pSoundName, const RwV3d* pPos = 0);

    /**
     * @brief Streams an extended audio file (background music, ambiance) via MCI.
     * @param pStreamName  Stream asset filename (e.g. "MenuMus", "BeaverM").
     * @param bLoop        TRUE to repeat indefinitely.
     */
    void PlayStream(const char* pStreamName, RwBool bLoop = FALSE);

    /**
     * @brief Stops currently streaming background music.
     */
    void StopStream();

    /**
     * @brief Immediately halts all playing sound channels and voice streams.
     */
    void StopAllVoices();

    // ---- Volume Group Control -----------------------------------------------

    /**
     * @brief Sets master volume gain for a specific group (0: Master, 1: Music, 2: FX).
     * @param groupIndex  Channel group index [0..2].
     * @param volume      Linear volume scalar [0.0 .. 1.0].
     */
    void SetVolumeGroup(RwUInt32 groupIndex, RwReal volume);

    /**
     * @brief Queries current volume level of a specified group.
     * @param groupIndex  Channel group index [0..2].
     * @return Volume gain [0.0 .. 1.0].
     */
    RwReal GetVolumeGroup(RwUInt32 groupIndex) const;

    // ---- 3D Listener Orientation --------------------------------------------

    /**
     * @brief Updates the virtual listener's 3D position and orientation vectors in RWS::Audio.
     * @param pMatrix  Transform matrix of the primary game camera.
     */
    void UpdateListenerPosition(const RwMatrix* pMatrix);

private:
    RwReal m_afVolumeGroups[3];     ///< Volume scalars for groups: 0=Master, 1=Music, 2=FX.
    RwMatrix m_ListenerMatrix;      ///< Current camera transform matrix for 3D panning.
    std::string m_SoundPath;        ///< Base directory path ("Sound\\Win32\\").
    std::string m_CurrentStream;    ///< Currently active stream filename.
};

#endif // AUDIOMANAGER_H

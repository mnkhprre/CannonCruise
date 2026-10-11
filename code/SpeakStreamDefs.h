#ifndef SPEAKSTREAMDEFS_H
#define SPEAKSTREAMDEFS_H

// ============================================================================
// CannonCruise - Speech Stream Definitions (SpeakStreamDefs.h)
// Original path: D:\Projects\CannonCruisePC\code\SpeakStreamDefs.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>

#define SPEAK_MAX_NUM_CATEGORIES 36

/**
 * @struct SSpeakCategoryInfo
 * @brief Metadata mapping category ID to dialogue count.
 */
struct SSpeakCategoryInfo
{
    RwUInt32 m_uCategory;   ///< Numerical category ID [1..36].
    RwUInt32 m_uCount;      ///< Total available voice variations in category.
};

/**
 * @class CSpeakStreamDefs
 * @brief Static lookup table defining voice line quotas per dialogue category.
 *
 * Populates and validates capacity tables for story lines, ambient chatter,
 * pirate barks, and enemy attack/death vocalisations.
 */
class CSpeakStreamDefs
{
public:
    /**
     * @brief Populates the category capacity tables with known voice quotas.
     */
    static void Initialize();

    /**
     * @brief Checks whether the speech definition database has been loaded.
     * @return TRUE if initialized and valid.
     */
    static RwBool IsValidSpeakSetLoaded();

    /**
     * @brief Returns the total number of audio takes available for a category.
     * @param category  Category ID [1..SPEAK_MAX_NUM_CATEGORIES].
     * @return Available voice line count, or 0 if category is out of range.
     */
    static RwUInt32 GetCategorySpeakCount(RwUInt32 category);

private:
    static RwUInt32 sm_anSpeaks[SPEAK_MAX_NUM_CATEGORIES + 1]; ///< Per-category line counts.
    static RwBool sm_bValidSpeakSetLoaded;                     ///< Initialization status flag.
};

#endif // SPEAKSTREAMDEFS_H

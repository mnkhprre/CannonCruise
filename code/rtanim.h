#ifndef RTANIM_H
#define RTANIM_H

#include "rwcore.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RtAnimAnimation
{
    void*  pCustomData;
    RwReal duration;
};
typedef struct RtAnimAnimation RtAnimAnimation;

RwBool RtAnimInitialize(void);
RwBool RtAnimRegisterInterpolationScheme(void *scheme);

#ifdef __cplusplus
}
#endif

#endif // RTANIM_H

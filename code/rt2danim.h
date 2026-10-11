#ifndef RT2DANIM_H
#define RT2DANIM_H

#include "rt2d.h"

#ifdef __cplusplus
extern "C" {
#endif

// 2D animation toolkit stub
struct Rt2dAnim;
typedef struct Rt2dAnim Rt2dAnim;

struct Rt2dMaestro;
typedef struct Rt2dMaestro Rt2dMaestro;

Rt2dMaestro* Rt2dMaestroCreate(void);
// 4.6r: .anm stream'inden create (parser: Rt2dMaestroStream.h)
Rt2dMaestro* Rt2dMaestroCreateFromMemory(const void *pData, RwUInt32 len);
RwBool       Rt2dMaestroDestroy(Rt2dMaestro* maestro);
void         Rt2dMaestroUpdate(Rt2dMaestro* maestro, RwReal deltaTime);
void         Rt2dMaestroRender(Rt2dMaestro* maestro);
// 4.6r menü durum makinesi: komut düğümleri arasında seçim
RwBool       Rt2dMaestroSelectNext(Rt2dMaestro* maestro);
RwBool       Rt2dMaestroSelectPrev(Rt2dMaestro* maestro);
const char*  Rt2dMaestroGetSelectedCommand(const Rt2dMaestro* maestro);

#ifdef __cplusplus
}
#endif

#endif // RT2DANIM_H

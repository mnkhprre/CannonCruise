// ============================================================================
// CannonCruise - librw Engine/Stream Shim (rw_librw.cpp)
// Original path: D:\Projects\CannonCruisePC\code\rw_librw.cpp
//
// rwstubs.cpp'deki NO-OP engine/stream sembollerinin GERÇEK implementasyonu:
// oyunun C API'si (Rw*/Rp* — RW SDK adlandırma) librw'nin C++ API'sine
// (rw::/rp::) bağlanır. Kapı 4'ün ilk adımı: gerçek .RWS asset akışı.
//
// Binary kaynakları: FUN_0050b640 RwEngineInit, FUN_00422960 RwEngineOpen,
// FUN_0050b340 RwEngineStart, FUN_0050b2f0 RwEngineClose, FUN_0050b3d0
// RwEngineStop, FUN_0050b600 RwEngineTerm; RwStream* ailesi + FindChunk;
// RpClumpStreamRead; RwTexDictionaryStreamRead.
//
// SAPMALAR (belgeli):
//   - NULL platform: cihaz yok (render sonraki iş kalemi — GL3/D3D9);
//     Engine::open(nil) parametre kabul etmez (device DEVICEOPEN no-op).
//   - RpWorldStreamRead: librw'de World streamRead YOK (BSP) → stub 0.
//   - RwMemoryFunctions eşlemesi: yalnız default memfuncs kullanılır
//     (parametre non-null ise yok sayılır).
//   - RwEngineSetConfig: librw karşılığı yok → TRUE (no-op).
// ============================================================================

#include "rwcore.h"
#include "rpworld.h"

#include <rw.h>

#include <cstdio>

namespace
{
    // RwStream sarmalayıcısı — librw StreamFile sahipliği
    struct ShimStream {
        rw::StreamFile file;
    };

    RwBool s_engineRunning = FALSE; // RwEngineIsRunning durumu
#ifdef RW_GL3
    GLFWwindow *s_glfwWindow = nullptr; // Engine::start create eder
    double s_lastMouseX = 0.0;          // mouse delta kalibrasyonu
    double s_lastMouseY = 0.0;
    RwBool s_mouseCalibrated = FALSE;
#endif
}

// ---------------------------------------------------------------------------
// Motor yaşam döngüsü — rw::Engine::init/open/start/stop/close/term
// ---------------------------------------------------------------------------
RwBool RwEngineInit(RwMemoryFunctions *memFuncs, RwUInt32 flags,
                    RwUInt32 resEntrySize) {
    (void)memFuncs; // default memfuncs (bkz. başlık sapması)
    (void)flags;
    (void)resEntrySize;
    if (rw::Engine::init() != TRUE) {
        return FALSE;
    }
    // Geometry MESH plugin — streamRead EXTENSION'ından meshHeader kurar;
    // GL3 instance/render meshHeader'sız çöker (NULL'da instance yoktu).
    rw::registerMeshPlugin();
    return TRUE;
}

RwBool RwEngineOpen(void) {
#ifdef RW_GL3
    // GL3: gl3-glfw cihazı EngineOpenParams ile pencere tanımlar
    // (openGLFW → glfwInit; pencere Engine::start'ta create edilir).
    rw::EngineOpenParams params;
    params.window = &s_glfwWindow;
    params.width = 1280;
    params.height = 720;
    params.windowtitle = "CannonCruise";
    return rw::Engine::open(&params) ? TRUE : FALSE;
#else
    // NULL platform: EngineOpenParams tanımsız — device açılışı no-op
    return rw::Engine::open(nil) ? TRUE : FALSE;
#endif
}

RwBool RwPlatformPollEvents(void) {
#ifdef RW_GL3
    glfwPollEvents();
    return TRUE;
#else
    return FALSE;
#endif
}

RwBool RwPlatformShouldClose(void) {
#ifdef RW_GL3
    return (s_glfwWindow != nullptr && glfwWindowShouldClose(s_glfwWindow))
               ? TRUE
               : FALSE;
#else
    return FALSE;
#endif
}

RwBool RwPlatformKeyDown(int key) {
#ifdef RW_GL3
    // GLFW_KEY_W == 'W' — ASCII kodlar doğrudan geçer.
    return (s_glfwWindow != nullptr && glfwGetKey(s_glfwWindow, key) == GLFW_PRESS)
               ? TRUE
               : FALSE;
#else
    (void)key;
    return FALSE;
#endif
}

RwBool RwPlatformSetMouseCapture(RwBool capture) {
#ifdef RW_GL3
    if (s_glfwWindow == nullptr) {
        return FALSE;
    }
    glfwSetInputMode(s_glfwWindow, GLFW_CURSOR,
                     (capture == TRUE) ? GLFW_CURSOR_DISABLED
                                       : GLFW_CURSOR_NORMAL);
    s_mouseCalibrated = FALSE; // yeniden kalibre (delta sıfırla)
    return TRUE;
#else
    (void)capture;
    return FALSE;
#endif
}

RwBool RwPlatformMouseDelta(int *pDX, int *pDY) {
#ifdef RW_GL3
    if (s_glfwWindow == nullptr || pDX == nullptr || pDY == nullptr) {
        return FALSE;
    }
    double x = 0.0;
    double y = 0.0;
    glfwGetCursorPos(s_glfwWindow, &x, &y);
    if (s_mouseCalibrated == FALSE) {
        s_lastMouseX = x;
        s_lastMouseY = y;
        s_mouseCalibrated = TRUE;
        *pDX = 0;
        *pDY = 0;
        return FALSE; // ilk kare: sıfır delta
    }
    *pDX = (int)(x - s_lastMouseX);
    *pDY = (int)(y - s_lastMouseY);
    s_lastMouseX = x;
    s_lastMouseY = y;
    return (*pDX != 0 || *pDY != 0) ? TRUE : FALSE;
#else
    (void)pDX;
    (void)pDY;
    return FALSE;
#endif
}

RwBool RwEngineStart(void) {
    if (!rw::Engine::start()) {
        return FALSE;
    }
    s_engineRunning = TRUE;
    return TRUE;
}

RwBool RwEngineStop(void) {
    rw::Engine::stop();
    s_engineRunning = FALSE;
    return TRUE;
}

RwBool RwEngineClose(void) {
    rw::Engine::close();
    return TRUE;
}

RwBool RwEngineTerm(void) {
    rw::Engine::term();
    return TRUE;
}

RwBool RwEngineIsRunning(void) { return s_engineRunning; }

RwBool RwEngineSetConfig(const RwEngineConfig *pConfig) {
    // librw'de near/fog globali yok — startup değerleri ({1e-5,1e-5,0.01f})
    // ileride camera kurulumuna taşınacak (belgeli no-op).
    (void)pConfig;
    return TRUE;
}

// ---------------------------------------------------------------------------
// Akışlar — rw::StreamFile
// ---------------------------------------------------------------------------
RwStream *RwStreamOpen(RwInt32 type, RwInt32 accessType, const void *pData) {
    if (type != rwSTREAMFILENAME || !pData) {
        return 0; // yalnız dosya yolu (rwSTREAMMEMORY henüz kullanılmıyor)
    }
    ShimStream *p = new ShimStream();
    const char *pMode = (accessType == rwSTREAMREAD) ? "rb" : "wb";
    if (!p->file.open(static_cast<const char *>(pData), pMode)) {
        delete p;
        return 0;
    }
    return reinterpret_cast<RwStream *>(p);
}

RwBool RwStreamClose(RwStream *stream, void *pData) {
    (void)pData;
    if (!stream) {
        return FALSE;
    }
    ShimStream *p = reinterpret_cast<ShimStream *>(stream);
    p->file.close();
    delete p;
    return TRUE;
}

RwUInt32 RwStreamRead(RwStream *stream, void *buffer, RwUInt32 length) {
    if (!stream || !buffer) {
        return 0;
    }
    ShimStream *p = reinterpret_cast<ShimStream *>(stream);
    return p->file.read8(buffer, length);
}

RwStream *RwStreamWrite(RwStream *stream, const void *buffer,
                        RwUInt32 length) {
    if (!stream || !buffer) {
        return 0;
    }
    ShimStream *p = reinterpret_cast<ShimStream *>(stream);
    p->file.write8(buffer, length);
    return stream;
}

RwUInt32 RwStreamSkip(RwStream *stream, RwUInt32 length) {
    if (!stream) {
        return 0;
    }
    ShimStream *p = reinterpret_cast<ShimStream *>(stream);
    p->file.seek(static_cast<rw::int32>(length), SEEK_CUR);
    return length;
}

RwUInt32 RwStreamTell(RwStream *stream) {
    if (!stream) {
        return 0;
    }
    ShimStream *p = reinterpret_cast<ShimStream *>(stream);
    return static_cast<RwUInt32>(p->file.tell());
}

RwBool RwStreamSeekAbs(RwStream *stream, RwUInt32 position) {
    if (!stream) {
        return FALSE;
    }
    ShimStream *p = reinterpret_cast<ShimStream *>(stream);
    p->file.seek(static_cast<rw::int32>(position), SEEK_SET);
    return TRUE;
}

RwBool RwStreamFindChunk(RwStream *stream, RwUInt32 type, RwUInt32 *lengthOut,
                         RwUInt32 *versionOut) {
    if (!stream) {
        return FALSE;
    }
    ShimStream *p = reinterpret_cast<ShimStream *>(stream);
    rw::uint32 length = 0;
    rw::uint32 version = 0;
    if (!rw::findChunk(&p->file, type, &length, &version)) {
        return FALSE;
    }
    if (lengthOut) {
        *lengthOut = length;
    }
    if (versionOut) {
        *versionOut = version;
    }
    return TRUE;
}

RwBool RwStreamReadChunkHeaderInfo(RwStream *stream,
                                   RwChunkHeaderInfo *chunkHeader) {
    if (!stream || !chunkHeader) {
        return FALSE;
    }
    ShimStream *p = reinterpret_cast<ShimStream *>(stream);
    rw::ChunkHeaderInfo header;
    if (!rw::readChunkHeaderInfo(&p->file, &header)) {
        return FALSE;
    }
    chunkHeader->type = header.type;
    chunkHeader->length = header.length;
    chunkHeader->version = header.version;
    chunkHeader->buildVersion = header.build;
    chunkHeader->isComplex = FALSE; // librw alan yok
    return TRUE;
}

// ---------------------------------------------------------------------------
// Varlık okuma — clump/txd (world: librw'de yok — stub)
// ---------------------------------------------------------------------------
RpClump *RpClumpStreamRead(RwStream *stream) {
    if (!stream) {
        return 0;
    }
    ShimStream *p = reinterpret_cast<ShimStream *>(stream);
    return reinterpret_cast<RpClump *>(rw::Clump::streamRead(&p->file));
}

RwTexDictionary *RwTexDictionaryStreamRead(RwStream *stream) {
    if (!stream) {
        return 0;
    }
    ShimStream *p = reinterpret_cast<ShimStream *>(stream);
    return reinterpret_cast<RwTexDictionary *>(
        rw::TexDictionary::streamRead(&p->file));
}

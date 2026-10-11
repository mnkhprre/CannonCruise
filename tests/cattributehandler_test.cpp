// SDK-independent regression test for RWS::CAttributeHandler.
// Covers instance registry and packet type helper predicates.
#include "attributehandler/cattributehandler.h"
#include <cstdio>

#define CHECK(condition) do { if (!(condition)) { \
    std::fprintf(stderr, "FAIL at line %d: %s\n", __LINE__, #condition); \
    return 1; } } while (0)

namespace
{
    struct RegisteredObject
    {
        int tag;
    };
} // namespace

int main()
{
    using RWS::CAttributeHandler;

    // -----------------------------------------------------------------
    // Initialize + tekil örnek
    // -----------------------------------------------------------------
    CAttributeHandler::Initialize();
    CHECK(CAttributeHandler::Instance() != 0);
    CHECK(CAttributeHandler::Instance() == CAttributeHandler::Instance());

    // -----------------------------------------------------------------
    // RegisterInstance / FindInstance / UnregisterInstance
    // -----------------------------------------------------------------
    RegisteredObject obj;
    obj.tag = 7;

    CAttributeHandler::RegisterInstance(1, &obj);
    CHECK(CAttributeHandler::FindInstance(1) == &obj);
    CHECK(CAttributeHandler::FindInstance(2) == 0);

    // Aynı id'ye yeniden kayıt güncellemeli
    RegisteredObject other;
    CAttributeHandler::RegisterInstance(1, &other);
    CHECK(CAttributeHandler::FindInstance(1) == &other);

    CAttributeHandler::UnregisterInstance(1);
    CHECK(CAttributeHandler::FindInstance(1) == 0);

    // -----------------------------------------------------------------
    // Paket tipi yardımcıları (tek bayt dizin işaretçileri)
    // -----------------------------------------------------------------
    const RwUInt8 instanceCreationPacket[] = { 0x01, 0xAA, 0xBB };
    const RwUInt8 dataPacket[]              = { 0x02, 0xAA };
    const RwUInt8 endChunkPacket[]          = { 0xFF };
    const RwUInt8 finishedPacket[]          = { 0x00 };

    CHECK(CAttributeHandler::IsInstanceCreationPacket(instanceCreationPacket) == TRUE);
    CHECK(CAttributeHandler::IsInstanceCreationPacket(dataPacket) == FALSE);
    CHECK(CAttributeHandler::IsInstanceCreationPacket(0) == FALSE);

    CHECK(CAttributeHandler::IsEndChunk(endChunkPacket) == TRUE);
    CHECK(CAttributeHandler::IsEndChunk(dataPacket) == FALSE);
    CHECK(CAttributeHandler::IsEndChunk(0) == TRUE);

    CHECK(CAttributeHandler::IsFinished(finishedPacket) == TRUE);
    CHECK(CAttributeHandler::IsFinished(dataPacket) == FALSE);
    CHECK(CAttributeHandler::IsFinished(0) == TRUE);

    // -----------------------------------------------------------------
    // Shutdown sonrası instance erişimi güvenli olmalı
    // -----------------------------------------------------------------
    CAttributeHandler::Shutdown();

    std::puts("PASS: attribute handler instance registry and packet predicates");
    return 0;
}

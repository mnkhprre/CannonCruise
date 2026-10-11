// SDK-independent regression test for the RWS event system.
// Covers RegisterMsg / LinkMsg / SendMsg / UnLinkMsg / lifecycle unlink.
#include "eventhandler/ceventhandler.h"
#include <cstdio>

#define CHECK(condition) do { if (!(condition)) { \
    std::fprintf(stderr, "FAIL at line %d: %s\n", __LINE__, #condition); \
    return 1; } } while (0)

using RWS::CEventId;
using RWS::CEventHandler;
using RWS::CMsg;

namespace
{
    int g_calls = 0;
    CEventId g_lastId = 0;
    const void* g_lastData = 0;

    class CountingHandler : public CEventHandler
    {
    public:
        virtual void HandleEvents(const CMsg& msg)
        {
            ++g_calls;
            g_lastId = msg.m_Id;
            g_lastData = msg.m_pData;
        }
    };
} // namespace

int main()
{
    CEventHandler::Initialize();

    // -----------------------------------------------------------------
    // RegisterMsg: benzersiz ve tekrar eden kayıt
    // -----------------------------------------------------------------
    const CEventId idA = CEventHandler::RegisterMsg("iMsgTestA");
    const CEventId idB = CEventHandler::RegisterMsg("iMsgTestB");
    CHECK(idA != 0);
    CHECK(idA != idB);
    // Aynı ad ikinci kez aynı kimliği döndürmeli
    CHECK(CEventHandler::RegisterMsg("iMsgTestA") == idA);

    // -----------------------------------------------------------------
    // LinkMsg + SendMsg dağıtımı
    // -----------------------------------------------------------------
    CountingHandler* pHandler = new CountingHandler();
    pHandler->LinkMsg(idA);

    const int payload = 42;
    g_calls = 0;
    CEventHandler::SendMsg(CMsg(idA, &payload));
    CHECK(g_calls == 1);
    CHECK(g_lastId == idA);
    CHECK(g_lastData == &payload);

    // Abone olunmayan olay çağrı üretmemeli
    g_calls = 0;
    CEventHandler::SendMsg(CMsg(idB, 0));
    CHECK(g_calls == 0);

    // -----------------------------------------------------------------
    // UnLinkMsg: abonelik kaldırıldıktan sonra dağıtım durur
    // -----------------------------------------------------------------
    pHandler->UnLinkMsg(idA);
    g_calls = 0;
    CEventHandler::SendMsg(CMsg(idA, 0));
    CHECK(g_calls == 0);

    // Yeniden link
    pHandler->LinkMsg(idA);
    g_calls = 0;
    CEventHandler::SendMsg(CMsg(idA, 0));
    CHECK(g_calls == 1);

    // -----------------------------------------------------------------
    // Yıkıcı tüm abonelikleri kaldırmalı (UnLinkAllProcess)
    // -----------------------------------------------------------------
    delete pHandler;
    g_calls = 0;
    CEventHandler::SendMsg(CMsg(idA, 0));
    CHECK(g_calls == 0);

    // -----------------------------------------------------------------
    // Çoklu abone: iki handler da almalı
    // -----------------------------------------------------------------
    CountingHandler* pH1 = new CountingHandler();
    CountingHandler* pH2 = new CountingHandler();
    pH1->LinkMsg(idB);
    pH2->LinkMsg(idB);
    g_calls = 0;
    CEventHandler::SendMsg(CMsg(idB, 0));
    CHECK(g_calls == 2);
    delete pH1;
    delete pH2;
    g_calls = 0;
    CEventHandler::SendMsg(CMsg(idB, 0));
    CHECK(g_calls == 0);

    // Kayıtlı sistem olayı sabitleri
    CHECK(RWS::iMsgRunningTick == 0x0001);
    CHECK(RWS::iMsgPhysicsUpdate == 0x0002);

    CEventHandler::Shutdown();
    std::puts("PASS: event register/link/send/unlink and handler lifecycle");
    return 0;
}

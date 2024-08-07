#ifdef _WIN64
#define NET_CHANNEL_IP_ADDRESS_OFFSET 0xd0  // 64-bit
#else
#define NET_CHANNEL_IP_ADDRESS_OFFSET 0x78  // 32-bit
#endif

#include "Server.h"

class ServerUtils {

    IGameFramework* m_pGameFramework;

public:

    void Init() {
        m_pGameFramework = gEnv->pGame->GetIGameFramework();
    }

    std::string CryChannelToIP(int channelId)
    {

        std::string result;

        if (!m_pGameFramework)
            return result;

        if (INetChannel* pNetChannel = m_pGameFramework->GetNetChannel(channelId))
        {
            const unsigned char* rawIP = (unsigned char*)pNetChannel + NET_CHANNEL_IP_ADDRESS_OFFSET;
            result = std::format("{}.{}.{}.{}", rawIP[3], rawIP[2], rawIP[1], rawIP[0]);
        }

        return result;
    }
};
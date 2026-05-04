#include "dlms/wrapper/wrapper_ports.hpp"

namespace dlms {
namespace wrapper {

bool IsNoStationWrapperPort(unsigned short port)
{
  return port == kNoStation;
}

bool IsClientWrapperPort(unsigned short port)
{
  return port >= 0x0001u && port <= 0x00ffu;
}

bool IsServerWrapperPort(unsigned short port)
{
  return port == kManagementLogicalDevice ||
         (port >= 0x0010u && port <= kAllStationBroadcast);
}

bool IsReservedClientWrapperPort(unsigned short port)
{
  return port == kNoStation ||
         port == kClientManagementProcess ||
         port == kPublicClient;
}

bool IsReservedServerWrapperPort(unsigned short port)
{
  return port == kNoStation ||
         port == kManagementLogicalDevice ||
         (port >= 0x0002u && port <= 0x000fu) ||
         port == kAllStationBroadcast;
}

bool IsOpenClientWrapperPort(unsigned short port)
{
  return (port >= 0x0002u && port <= 0x000fu) ||
         (port >= 0x0011u && port <= 0x00ffu);
}

bool IsOpenServerWrapperPort(unsigned short port)
{
  return port >= 0x0010u && port <= 0x007eu;
}

} // namespace wrapper
} // namespace dlms

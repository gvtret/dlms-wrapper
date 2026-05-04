#pragma once

namespace dlms {
namespace wrapper {

const unsigned short kWrapperVersion = 0x0001u;
const unsigned short kWrapperHeaderSize = 8u;
const unsigned int kMaximumWrapperDataLength = 0xffffu;
const unsigned int kMaximumWrapperFrameLength =
  kWrapperHeaderSize + kMaximumWrapperDataLength;

const unsigned short kNoStation = 0x0000u;

const unsigned short kClientManagementProcess = 0x0001u;
const unsigned short kPublicClient = 0x0010u;

const unsigned short kManagementLogicalDevice = 0x0001u;
const unsigned short kAllStationBroadcast = 0x007fu;

bool IsNoStationWrapperPort(unsigned short port);

bool IsClientWrapperPort(unsigned short port);
bool IsServerWrapperPort(unsigned short port);

bool IsReservedClientWrapperPort(unsigned short port);
bool IsReservedServerWrapperPort(unsigned short port);

bool IsOpenClientWrapperPort(unsigned short port);
bool IsOpenServerWrapperPort(unsigned short port);

} // namespace wrapper
} // namespace dlms

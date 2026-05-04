#include "dlms/wrapper/wrapper_ports.hpp"

#include <gtest/gtest.h>

namespace {

using namespace dlms::wrapper;

TEST(WrapperPortsTest, ConstantsMatchDlmsCosemProfile)
{
  EXPECT_EQ(0x0001u, kWrapperVersion);
  EXPECT_EQ(8u, kWrapperHeaderSize);
  EXPECT_EQ(0xffffu, kMaximumWrapperDataLength);
  EXPECT_EQ(0x10007u, kMaximumWrapperFrameLength);

  EXPECT_EQ(0x0000u, kNoStation);
  EXPECT_EQ(0x0001u, kClientManagementProcess);
  EXPECT_EQ(0x0010u, kPublicClient);
  EXPECT_EQ(0x0001u, kManagementLogicalDevice);
  EXPECT_EQ(0x007fu, kAllStationBroadcast);
}

TEST(WrapperPortsTest, RecognizesClientPorts)
{
  EXPECT_FALSE(IsClientWrapperPort(kNoStation));
  EXPECT_TRUE(IsClientWrapperPort(kClientManagementProcess));
  EXPECT_TRUE(IsClientWrapperPort(0x0002u));
  EXPECT_TRUE(IsClientWrapperPort(kPublicClient));
  EXPECT_TRUE(IsClientWrapperPort(0x00ffu));
  EXPECT_FALSE(IsClientWrapperPort(0x0100u));
}

TEST(WrapperPortsTest, RecognizesServerPorts)
{
  EXPECT_FALSE(IsServerWrapperPort(kNoStation));
  EXPECT_TRUE(IsServerWrapperPort(kManagementLogicalDevice));
  EXPECT_FALSE(IsServerWrapperPort(0x000fu));
  EXPECT_TRUE(IsServerWrapperPort(0x0010u));
  EXPECT_TRUE(IsServerWrapperPort(0x007eu));
  EXPECT_TRUE(IsServerWrapperPort(kAllStationBroadcast));
  EXPECT_FALSE(IsServerWrapperPort(0x0080u));
}

TEST(WrapperPortsTest, ClassifiesClientReservedAndOpenRanges)
{
  EXPECT_TRUE(IsReservedClientWrapperPort(kNoStation));
  EXPECT_TRUE(IsReservedClientWrapperPort(kClientManagementProcess));
  EXPECT_TRUE(IsReservedClientWrapperPort(kPublicClient));
  EXPECT_FALSE(IsReservedClientWrapperPort(0x0002u));
  EXPECT_FALSE(IsReservedClientWrapperPort(0x0011u));

  EXPECT_FALSE(IsOpenClientWrapperPort(kNoStation));
  EXPECT_FALSE(IsOpenClientWrapperPort(kClientManagementProcess));
  EXPECT_TRUE(IsOpenClientWrapperPort(0x0002u));
  EXPECT_TRUE(IsOpenClientWrapperPort(0x000fu));
  EXPECT_FALSE(IsOpenClientWrapperPort(kPublicClient));
  EXPECT_TRUE(IsOpenClientWrapperPort(0x0011u));
  EXPECT_TRUE(IsOpenClientWrapperPort(0x00ffu));
}

TEST(WrapperPortsTest, ClassifiesServerReservedAndOpenRanges)
{
  EXPECT_TRUE(IsReservedServerWrapperPort(kNoStation));
  EXPECT_TRUE(IsReservedServerWrapperPort(kManagementLogicalDevice));
  EXPECT_TRUE(IsReservedServerWrapperPort(0x0002u));
  EXPECT_TRUE(IsReservedServerWrapperPort(0x000fu));
  EXPECT_FALSE(IsReservedServerWrapperPort(0x0010u));
  EXPECT_FALSE(IsReservedServerWrapperPort(0x007eu));
  EXPECT_TRUE(IsReservedServerWrapperPort(kAllStationBroadcast));

  EXPECT_FALSE(IsOpenServerWrapperPort(kNoStation));
  EXPECT_FALSE(IsOpenServerWrapperPort(kManagementLogicalDevice));
  EXPECT_FALSE(IsOpenServerWrapperPort(0x000fu));
  EXPECT_TRUE(IsOpenServerWrapperPort(0x0010u));
  EXPECT_TRUE(IsOpenServerWrapperPort(0x007eu));
  EXPECT_FALSE(IsOpenServerWrapperPort(kAllStationBroadcast));
}

} // namespace

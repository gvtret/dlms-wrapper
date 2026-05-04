#include "dlms/wrapper/wrapper_codec.hpp"
#include "dlms/wrapper/wrapper_ports.hpp"

#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

namespace {

using namespace dlms::wrapper;

TEST(WrapperHeaderCodecTest, EncodesHeaderBigEndian)
{
  WrapperHeader header;
  header.version = kWrapperVersion;
  header.sourcePort = kPublicClient;
  header.destinationPort = kManagementLogicalDevice;
  header.dataLength = 0x1234u;

  std::uint8_t output[8] = {};
  std::size_t writtenSize = 0;

  ASSERT_EQ(WrapperStatus::Ok,
            EncodeWrapperHeader(header, output, sizeof(output), writtenSize));

  EXPECT_EQ(8u, writtenSize);
  EXPECT_EQ(0x00u, output[0]);
  EXPECT_EQ(0x01u, output[1]);
  EXPECT_EQ(0x00u, output[2]);
  EXPECT_EQ(0x10u, output[3]);
  EXPECT_EQ(0x00u, output[4]);
  EXPECT_EQ(0x01u, output[5]);
  EXPECT_EQ(0x12u, output[6]);
  EXPECT_EQ(0x34u, output[7]);
}

TEST(WrapperHeaderCodecTest, DecodesHeaderBigEndian)
{
  const std::uint8_t input[] = {
    0x00, 0x01,
    0x00, 0x10,
    0x00, 0x01,
    0x12, 0x34
  };

  WrapperHeader header;
  ASSERT_EQ(WrapperStatus::Ok,
            DecodeWrapperHeader(input, sizeof(input), header));

  EXPECT_EQ(kWrapperVersion, header.version);
  EXPECT_EQ(kPublicClient, header.sourcePort);
  EXPECT_EQ(kManagementLogicalDevice, header.destinationPort);
  EXPECT_EQ(0x1234u, header.dataLength);
}

TEST(WrapperHeaderCodecTest, RejectsInvalidVersion)
{
  WrapperHeader header;
  header.version = 0x0002u;
  header.sourcePort = kPublicClient;
  header.destinationPort = kManagementLogicalDevice;
  header.dataLength = 0u;

  std::uint8_t output[8] = {};
  std::size_t writtenSize = 0;
  EXPECT_EQ(WrapperStatus::InvalidVersion,
            EncodeWrapperHeader(header, output, sizeof(output), writtenSize));

  const std::uint8_t input[] = {
    0x00, 0x02,
    0x00, 0x10,
    0x00, 0x01,
    0x00, 0x00
  };
  EXPECT_EQ(WrapperStatus::InvalidVersion,
            DecodeWrapperHeader(input, sizeof(input), header));
}

TEST(WrapperHeaderCodecTest, ReportsShortHeaderAsNeedMoreData)
{
  const std::uint8_t input[] = {0x00, 0x01, 0x00};

  WrapperHeader header;
  EXPECT_EQ(WrapperStatus::NeedMoreData,
            DecodeWrapperHeader(input, sizeof(input), header));
}

TEST(WrapperHeaderCodecTest, ReportsSmallOutputBuffer)
{
  WrapperHeader header;
  header.version = kWrapperVersion;
  header.sourcePort = kPublicClient;
  header.destinationPort = kManagementLogicalDevice;
  header.dataLength = 0u;

  std::uint8_t output[7] = {};
  std::size_t writtenSize = 1;
  EXPECT_EQ(WrapperStatus::OutputBufferTooSmall,
            EncodeWrapperHeader(header, output, sizeof(output), writtenSize));
  EXPECT_EQ(0u, writtenSize);
}

TEST(WrapperHeaderCodecTest, ValidatesDeclaredDataLengthAgainstLimitsAndInput)
{
  WrapperHeader header;
  header.version = kWrapperVersion;
  header.sourcePort = kPublicClient;
  header.destinationPort = kManagementLogicalDevice;
  header.dataLength = 2u;

  WrapperCodecLimits limits = DefaultWrapperCodecLimits();

  EXPECT_EQ(WrapperStatus::NeedMoreData,
            ValidateWrapperHeader(header, limits, 1u));
  EXPECT_EQ(WrapperStatus::Ok,
            ValidateWrapperHeader(header, limits, 2u));
  EXPECT_EQ(WrapperStatus::InvalidLength,
            ValidateWrapperHeader(header, limits, 3u));

  limits.maximumDataSize = 1u;
  EXPECT_EQ(WrapperStatus::DataTooLarge,
            ValidateWrapperHeader(header, limits, 2u));

  limits.maximumDataSize = 2u;
  limits.maximumFrameSize = 9u;
  EXPECT_EQ(WrapperStatus::FrameTooLarge,
            ValidateWrapperHeader(header, limits, 2u));
}

TEST(WrapperWpduEncoderTest, EncodesEmptyData)
{
  WrapperFrame frame;
  frame.sourcePort = kPublicClient;
  frame.destinationPort = kManagementLogicalDevice;
  frame.data = 0;
  frame.dataSize = 0;

  std::vector<std::uint8_t> output;
  ASSERT_EQ(WrapperStatus::Ok,
            EncodeWpdu(frame, DefaultWrapperCodecLimits(), output));

  const std::uint8_t expected[] = {
    0x00, 0x01,
    0x00, 0x10,
    0x00, 0x01,
    0x00, 0x00
  };
  EXPECT_EQ(std::vector<std::uint8_t>(expected, expected + sizeof(expected)),
            output);
}

TEST(WrapperWpduEncoderTest, EncodesApduPayload)
{
  const std::uint8_t apdu[] = {0xc0, 0x01, 0x81, 0x00};

  WrapperFrame frame;
  frame.sourcePort = kPublicClient;
  frame.destinationPort = kManagementLogicalDevice;
  frame.data = apdu;
  frame.dataSize = sizeof(apdu);

  std::uint8_t output[12] = {};
  std::size_t writtenSize = 0;
  ASSERT_EQ(WrapperStatus::Ok,
            EncodeWpduToBuffer(frame,
                               DefaultWrapperCodecLimits(),
                               output,
                               sizeof(output),
                               writtenSize));

  const std::uint8_t expected[] = {
    0x00, 0x01,
    0x00, 0x10,
    0x00, 0x01,
    0x00, 0x04,
    0xc0, 0x01, 0x81, 0x00
  };
  EXPECT_EQ(sizeof(expected), writtenSize);
  EXPECT_EQ(std::vector<std::uint8_t>(expected, expected + sizeof(expected)),
            std::vector<std::uint8_t>(output, output + writtenSize));
}

TEST(WrapperWpduEncoderTest, PreservesPayloadByte7e)
{
  const std::uint8_t apdu[] = {0xc0, 0x7e, 0x00};

  WrapperFrame frame;
  frame.sourcePort = kPublicClient;
  frame.destinationPort = kManagementLogicalDevice;
  frame.data = apdu;
  frame.dataSize = sizeof(apdu);

  std::vector<std::uint8_t> output;
  ASSERT_EQ(WrapperStatus::Ok,
            EncodeWpdu(frame, DefaultWrapperCodecLimits(), output));

  ASSERT_EQ(11u, output.size());
  EXPECT_EQ(0xc0u, output[8]);
  EXPECT_EQ(0x7eu, output[9]);
  EXPECT_EQ(0x00u, output[10]);
}

TEST(WrapperWpduEncoderTest, ReportsSmallOutputBuffer)
{
  const std::uint8_t apdu[] = {0xc0, 0x01};

  WrapperFrame frame;
  frame.sourcePort = kPublicClient;
  frame.destinationPort = kManagementLogicalDevice;
  frame.data = apdu;
  frame.dataSize = sizeof(apdu);

  std::uint8_t output[9] = {};
  std::size_t writtenSize = 1;
  EXPECT_EQ(WrapperStatus::OutputBufferTooSmall,
            EncodeWpduToBuffer(frame,
                               DefaultWrapperCodecLimits(),
                               output,
                               sizeof(output),
                               writtenSize));
  EXPECT_EQ(0u, writtenSize);
}

TEST(WrapperWpduEncoderTest, RejectsInvalidArgumentsAndPorts)
{
  WrapperFrame frame;
  frame.sourcePort = kPublicClient;
  frame.destinationPort = kManagementLogicalDevice;
  frame.data = 0;
  frame.dataSize = 1;

  std::vector<std::uint8_t> output;
  EXPECT_EQ(WrapperStatus::InvalidArgument,
            EncodeWpdu(frame, DefaultWrapperCodecLimits(), output));

  const std::uint8_t apdu[] = {0xc0};
  frame.data = apdu;
  frame.dataSize = sizeof(apdu);
  frame.sourcePort = kNoStation;
  EXPECT_EQ(WrapperStatus::InvalidSourcePort,
            EncodeWpdu(frame, DefaultWrapperCodecLimits(), output));

  frame.sourcePort = kPublicClient;
  frame.destinationPort = kNoStation;
  EXPECT_EQ(WrapperStatus::InvalidDestinationPort,
            EncodeWpdu(frame, DefaultWrapperCodecLimits(), output));
}

TEST(WrapperWpduEncoderTest, EnforcesLimits)
{
  const std::uint8_t apdu[] = {0xc0, 0x01};

  WrapperFrame frame;
  frame.sourcePort = kPublicClient;
  frame.destinationPort = kManagementLogicalDevice;
  frame.data = apdu;
  frame.dataSize = sizeof(apdu);

  WrapperCodecLimits limits = DefaultWrapperCodecLimits();
  limits.maximumDataSize = 1u;

  std::vector<std::uint8_t> output;
  EXPECT_EQ(WrapperStatus::DataTooLarge,
            EncodeWpdu(frame, limits, output));

  limits.maximumDataSize = 2u;
  limits.maximumFrameSize = 9u;
  EXPECT_EQ(WrapperStatus::FrameTooLarge,
            EncodeWpdu(frame, limits, output));
}

} // namespace

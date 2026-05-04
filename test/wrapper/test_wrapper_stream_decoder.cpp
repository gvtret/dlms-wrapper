#include "dlms/wrapper/wrapper_stream_decoder.hpp"

#include "dlms/wrapper/wrapper_ports.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

namespace {

using namespace dlms::wrapper;

WrapperStreamDecoder MakeDecoder()
{
  WrapperStreamDecoderOptions options;
  options.limits = DefaultWrapperCodecLimits();
  return WrapperStreamDecoder(options);
}

std::vector<std::uint8_t> EncodePayload(const std::vector<std::uint8_t>& payload)
{
  WrapperFrame frame;
  frame.sourcePort = kPublicClient;
  frame.destinationPort = kManagementLogicalDevice;
  frame.data = payload.empty() ? 0 : &payload[0];
  frame.dataSize = payload.size();

  std::vector<std::uint8_t> encoded;
  EXPECT_EQ(WrapperStatus::Ok,
            EncodeWpdu(frame, DefaultWrapperCodecLimits(), encoded));
  return encoded;
}

TEST(WrapperStreamDecoderTest, PushFullWpdu)
{
  const std::uint8_t payload[] = {0xc0, 0x01};
  const std::vector<std::uint8_t> encoded =
    EncodePayload(std::vector<std::uint8_t>(payload, payload + sizeof(payload)));

  WrapperStreamDecoder decoder = MakeDecoder();
  std::vector<WrapperFrameBuffer> frames;
  ASSERT_EQ(WrapperStatus::Ok, decoder.Push(&encoded[0], encoded.size(), frames));

  ASSERT_EQ(1u, frames.size());
  EXPECT_EQ(std::vector<std::uint8_t>(payload, payload + sizeof(payload)),
            frames[0].data);
}

TEST(WrapperStreamDecoderTest, PushHeaderThenData)
{
  const std::uint8_t payload[] = {0xc0, 0x01, 0x81};
  const std::vector<std::uint8_t> encoded =
    EncodePayload(std::vector<std::uint8_t>(payload, payload + sizeof(payload)));

  WrapperStreamDecoder decoder = MakeDecoder();
  std::vector<WrapperFrameBuffer> frames;

  EXPECT_EQ(WrapperStatus::NeedMoreData,
            decoder.Push(&encoded[0], kWrapperHeaderSize, frames));
  EXPECT_TRUE(frames.empty());

  ASSERT_EQ(WrapperStatus::Ok,
            decoder.Push(&encoded[kWrapperHeaderSize],
                         encoded.size() - kWrapperHeaderSize,
                         frames));
  ASSERT_EQ(1u, frames.size());
  EXPECT_EQ(std::vector<std::uint8_t>(payload, payload + sizeof(payload)),
            frames[0].data);
}

TEST(WrapperStreamDecoderTest, PushByteByByte)
{
  const std::uint8_t payload[] = {0xc0, 0x01, 0x7e, 0x00};
  const std::vector<std::uint8_t> encoded =
    EncodePayload(std::vector<std::uint8_t>(payload, payload + sizeof(payload)));

  WrapperStreamDecoder decoder = MakeDecoder();
  std::vector<WrapperFrameBuffer> frames;

  for (std::size_t i = 0; i + 1 < encoded.size(); ++i) {
    EXPECT_EQ(WrapperStatus::NeedMoreData, decoder.Push(&encoded[i], 1u, frames));
    EXPECT_TRUE(frames.empty());
  }

  ASSERT_EQ(WrapperStatus::Ok,
            decoder.Push(&encoded[encoded.size() - 1], 1u, frames));
  ASSERT_EQ(1u, frames.size());
  EXPECT_EQ(std::vector<std::uint8_t>(payload, payload + sizeof(payload)),
            frames[0].data);
}

TEST(WrapperStreamDecoderTest, PushMultipleWpdus)
{
  const std::uint8_t firstPayload[] = {0xc0, 0x01};
  const std::uint8_t secondPayload[] = {0xc1, 0x02, 0x03};

  std::vector<std::uint8_t> stream = EncodePayload(
    std::vector<std::uint8_t>(firstPayload, firstPayload + sizeof(firstPayload)));
  const std::vector<std::uint8_t> second = EncodePayload(
    std::vector<std::uint8_t>(secondPayload, secondPayload + sizeof(secondPayload)));
  stream.insert(stream.end(), second.begin(), second.end());

  WrapperStreamDecoder decoder = MakeDecoder();
  std::vector<WrapperFrameBuffer> frames;

  ASSERT_EQ(WrapperStatus::Ok, decoder.Push(&stream[0], stream.size(), frames));
  ASSERT_EQ(2u, frames.size());
  EXPECT_EQ(std::vector<std::uint8_t>(firstPayload,
                                      firstPayload + sizeof(firstPayload)),
            frames[0].data);
  EXPECT_EQ(std::vector<std::uint8_t>(secondPayload,
                                      secondPayload + sizeof(secondPayload)),
            frames[1].data);
}

TEST(WrapperStreamDecoderTest, LeavesPartialSecondWpduBuffered)
{
  const std::uint8_t firstPayload[] = {0xc0};
  const std::uint8_t secondPayload[] = {0xc1, 0x02};

  std::vector<std::uint8_t> stream = EncodePayload(
    std::vector<std::uint8_t>(firstPayload, firstPayload + sizeof(firstPayload)));
  const std::vector<std::uint8_t> second = EncodePayload(
    std::vector<std::uint8_t>(secondPayload, secondPayload + sizeof(secondPayload)));
  stream.insert(stream.end(), second.begin(), second.begin() + 5);

  WrapperStreamDecoder decoder = MakeDecoder();
  std::vector<WrapperFrameBuffer> frames;

  ASSERT_EQ(WrapperStatus::Ok, decoder.Push(&stream[0], stream.size(), frames));
  ASSERT_EQ(1u, frames.size());

  ASSERT_EQ(WrapperStatus::Ok,
            decoder.Push(&second[5], second.size() - 5, frames));
  ASSERT_EQ(1u, frames.size());
  EXPECT_EQ(std::vector<std::uint8_t>(secondPayload,
                                      secondPayload + sizeof(secondPayload)),
            frames[0].data);
}

TEST(WrapperStreamDecoderTest, RejectsInvalidVersionAndCanReset)
{
  const std::uint8_t invalid[] = {
    0x00, 0x02,
    0x00, 0x10,
    0x00, 0x01,
    0x00, 0x00
  };

  WrapperStreamDecoder decoder = MakeDecoder();
  std::vector<WrapperFrameBuffer> frames;

  EXPECT_EQ(WrapperStatus::InvalidVersion,
            decoder.Push(invalid, sizeof(invalid), frames));

  decoder.Reset();

  const std::uint8_t payload[] = {0xc0};
  const std::vector<std::uint8_t> encoded =
    EncodePayload(std::vector<std::uint8_t>(payload, payload + sizeof(payload)));

  ASSERT_EQ(WrapperStatus::Ok, decoder.Push(&encoded[0], encoded.size(), frames));
  ASSERT_EQ(1u, frames.size());
}

TEST(WrapperStreamDecoderTest, EnforcesFrameTooLarge)
{
  const std::uint8_t payload[] = {0xc0, 0x01};
  const std::vector<std::uint8_t> encoded =
    EncodePayload(std::vector<std::uint8_t>(payload, payload + sizeof(payload)));

  WrapperStreamDecoderOptions options;
  options.limits = DefaultWrapperCodecLimits();
  options.limits.maximumFrameSize = 9u;
  WrapperStreamDecoder decoder(options);

  std::vector<WrapperFrameBuffer> frames;
  EXPECT_EQ(WrapperStatus::FrameTooLarge,
            decoder.Push(&encoded[0], encoded.size(), frames));
}

TEST(WrapperStreamDecoderTest, RejectsNullInputWithNonZeroSize)
{
  WrapperStreamDecoder decoder = MakeDecoder();
  std::vector<WrapperFrameBuffer> frames;

  EXPECT_EQ(WrapperStatus::InvalidArgument, decoder.Push(0, 1u, frames));
}

} // namespace

#include "dlms/wrapper/wrapper_codec.hpp"
#include "dlms/wrapper/wrapper_ports.hpp"
#include "dlms/wrapper/wrapper_stream_decoder.hpp"

#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

namespace {

using namespace dlms::wrapper;

const std::uint8_t kTraceAarq[] = {
  0x00, 0x01, 0x00, 0x30, 0x00, 0x10, 0x00, 0x40,
  0x60, 0x3e, 0xa1, 0x09, 0x06, 0x07, 0x60, 0x85,
  0x74, 0x05, 0x08, 0x01, 0x01, 0x8a, 0x02, 0x07,
  0x80, 0x8b, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08,
  0x02, 0x02, 0xac, 0x12, 0x80, 0x10, 0x13, 0x72,
  0x1b, 0x6d, 0x4d, 0x1d, 0x60, 0x22, 0x31, 0x02,
  0x63, 0x48, 0x03, 0x21, 0x5f, 0x79, 0xbe, 0x10,
  0x04, 0x0e, 0x01, 0x00, 0x00, 0x00, 0x06, 0x5f,
  0x1f, 0x04, 0x00, 0x62, 0x1e, 0x5d, 0x02, 0x00
};

const std::uint8_t kTraceAare[] = {
  0x00, 0x01, 0x00, 0x10, 0x00, 0x30, 0x00, 0x50,
  0x61, 0x4e, 0x80, 0x02, 0x02, 0x84, 0xa1, 0x09,
  0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01,
  0x01, 0xa2, 0x03, 0x02, 0x01, 0x00, 0xa3, 0x05,
  0xa1, 0x03, 0x02, 0x01, 0x0e, 0x88, 0x02, 0x07,
  0x80, 0x89, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08,
  0x02, 0x02, 0xaa, 0x12, 0x80, 0x10, 0xc6, 0x69,
  0x73, 0x51, 0xff, 0x4a, 0xec, 0x29, 0xcd, 0xba,
  0xab, 0xf2, 0xfb, 0xe3, 0x46, 0x7c, 0xbe, 0x10,
  0x04, 0x0e, 0x08, 0x00, 0x06, 0x5f, 0x1f, 0x04,
  0x00, 0x40, 0x18, 0x1d, 0x02, 0x00, 0x00, 0x07
};

const std::uint8_t kTraceGetRequest[] = {
  0x00, 0x01, 0x00, 0x30, 0x00, 0x10, 0x00, 0x0d,
  0xc0, 0x01, 0x81, 0x00, 0x07, 0x01, 0x00, 0x63,
  0x01, 0x00, 0xff, 0x04, 0x00
};

const std::uint8_t kTraceGetResponse[] = {
  0x00, 0x01, 0x00, 0x10, 0x00, 0x30, 0x00, 0x09,
  0xc4, 0x01, 0x81, 0x00, 0x06, 0x00, 0x00, 0x07,
  0x08
};

std::vector<std::uint8_t> CopyBytes(const std::uint8_t* data, std::size_t size)
{
  return std::vector<std::uint8_t>(data, data + size);
}

void ExpectTraceFrameDecodes(
  const std::uint8_t* encoded,
  std::size_t encodedSize,
  std::uint16_t sourcePort,
  std::uint16_t destinationPort,
  std::size_t dataSize,
  std::uint8_t firstApduByte)
{
  WrapperFrameBuffer decoded;
  ASSERT_EQ(WrapperStatus::Ok,
            DecodeWpdu(encoded, encodedSize, DefaultWrapperCodecLimits(), decoded));

  EXPECT_EQ(sourcePort, decoded.sourcePort);
  EXPECT_EQ(destinationPort, decoded.destinationPort);
  ASSERT_EQ(dataSize, decoded.data.size());
  EXPECT_EQ(firstApduByte, decoded.data[0]);

  WrapperFrame frame;
  frame.sourcePort = decoded.sourcePort;
  frame.destinationPort = decoded.destinationPort;
  frame.data = &decoded.data[0];
  frame.dataSize = decoded.data.size();

  std::vector<std::uint8_t> reencoded;
  ASSERT_EQ(WrapperStatus::Ok,
            EncodeWpdu(frame, DefaultWrapperCodecLimits(), reencoded));
  EXPECT_EQ(CopyBytes(encoded, encodedSize), reencoded);
}

TEST(WrapperTraceVectorTest, DecodesAndReencodesAssociationFrames)
{
  ExpectTraceFrameDecodes(kTraceAarq,
                          sizeof(kTraceAarq),
                          0x0030u,
                          kPublicClient,
                          0x0040u,
                          0x60u);

  ExpectTraceFrameDecodes(kTraceAare,
                          sizeof(kTraceAare),
                          kPublicClient,
                          0x0030u,
                          0x0050u,
                          0x61u);
}

TEST(WrapperTraceVectorTest, DecodesAndReencodesGetFrames)
{
  ExpectTraceFrameDecodes(kTraceGetRequest,
                          sizeof(kTraceGetRequest),
                          0x0030u,
                          kPublicClient,
                          0x000du,
                          0xc0u);

  ExpectTraceFrameDecodes(kTraceGetResponse,
                          sizeof(kTraceGetResponse),
                          kPublicClient,
                          0x0030u,
                          0x0009u,
                          0xc4u);
}

TEST(WrapperTraceVectorTest, StreamDecoderUsesTraceDataLengths)
{
  std::vector<std::uint8_t> stream = CopyBytes(kTraceAarq, sizeof(kTraceAarq));
  stream.insert(stream.end(), kTraceAare, kTraceAare + sizeof(kTraceAare));
  stream.insert(stream.end(), kTraceGetRequest, kTraceGetRequest + sizeof(kTraceGetRequest));
  stream.insert(stream.end(), kTraceGetResponse, kTraceGetResponse + sizeof(kTraceGetResponse));

  WrapperStreamDecoderOptions options;
  options.limits = DefaultWrapperCodecLimits();
  WrapperStreamDecoder decoder(options);

  std::vector<WrapperFrameBuffer> frames;
  ASSERT_EQ(WrapperStatus::Ok,
            decoder.Push(&stream[0], stream.size(), frames));

  ASSERT_EQ(4u, frames.size());
  EXPECT_EQ(0x0040u, frames[0].data.size());
  EXPECT_EQ(0x0050u, frames[1].data.size());
  EXPECT_EQ(0x000du, frames[2].data.size());
  EXPECT_EQ(0x0009u, frames[3].data.size());
}

} // namespace

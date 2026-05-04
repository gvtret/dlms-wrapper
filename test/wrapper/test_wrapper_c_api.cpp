#include "dlms/wrapper/wrapper_c_api.h"

#include <cstddef>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

extern "C" int dlms_wrapper_c_header_compiles_as_c(void);

namespace {

TEST(WrapperCApiTest, HeaderCompilesAsC)
{
  EXPECT_EQ(0, dlms_wrapper_c_header_compiles_as_c());
}

TEST(WrapperCApiTest, EncodesWpduIntoCallerBuffer)
{
  const std::uint8_t apdu[] = {0xc0, 0x01};
  std::uint8_t output[10] = {};
  std::size_t writtenSize = 0;

  ASSERT_EQ(DLMS_WRAPPER_STATUS_OK,
            dlms_wrapper_encode_wpdu(0x0010u,
                                     0x0001u,
                                     apdu,
                                     sizeof(apdu),
                                     output,
                                     sizeof(output),
                                     &writtenSize));

  const std::uint8_t expected[] = {
    0x00, 0x01,
    0x00, 0x10,
    0x00, 0x01,
    0x00, 0x02,
    0xc0, 0x01
  };
  EXPECT_EQ(sizeof(expected), writtenSize);
  EXPECT_EQ(std::vector<std::uint8_t>(expected, expected + sizeof(expected)),
            std::vector<std::uint8_t>(output, output + writtenSize));
}

TEST(WrapperCApiTest, ReportsSmallEncodeOutputBuffer)
{
  const std::uint8_t apdu[] = {0xc0, 0x01};
  std::uint8_t output[9] = {};
  std::size_t writtenSize = 1;

  EXPECT_EQ(DLMS_WRAPPER_STATUS_OUTPUT_BUFFER_TOO_SMALL,
            dlms_wrapper_encode_wpdu(0x0010u,
                                     0x0001u,
                                     apdu,
                                     sizeof(apdu),
                                     output,
                                     sizeof(output),
                                     &writtenSize));
  EXPECT_EQ(0u, writtenSize);
}

TEST(WrapperCApiTest, DecodesWpduIntoCallerBuffer)
{
  const std::uint8_t wpdu[] = {
    0x00, 0x01,
    0x00, 0x10,
    0x00, 0x01,
    0x00, 0x02,
    0xc0, 0x01
  };

  std::uint16_t sourcePort = 0;
  std::uint16_t destinationPort = 0;
  std::uint8_t data[2] = {};
  std::size_t dataSize = 0;

  ASSERT_EQ(DLMS_WRAPPER_STATUS_OK,
            dlms_wrapper_decode_wpdu(wpdu,
                                     sizeof(wpdu),
                                     &sourcePort,
                                     &destinationPort,
                                     data,
                                     sizeof(data),
                                     &dataSize));

  EXPECT_EQ(0x0010u, sourcePort);
  EXPECT_EQ(0x0001u, destinationPort);
  EXPECT_EQ(2u, dataSize);
  EXPECT_EQ(0xc0u, data[0]);
  EXPECT_EQ(0x01u, data[1]);
}

TEST(WrapperCApiTest, ReportsSmallDecodeOutputBuffer)
{
  const std::uint8_t wpdu[] = {
    0x00, 0x01,
    0x00, 0x10,
    0x00, 0x01,
    0x00, 0x02,
    0xc0, 0x01
  };

  std::uint16_t sourcePort = 0;
  std::uint16_t destinationPort = 0;
  std::uint8_t data[1] = {};
  std::size_t dataSize = 0;

  EXPECT_EQ(DLMS_WRAPPER_STATUS_OUTPUT_BUFFER_TOO_SMALL,
            dlms_wrapper_decode_wpdu(wpdu,
                                     sizeof(wpdu),
                                     &sourcePort,
                                     &destinationPort,
                                     data,
                                     sizeof(data),
                                     &dataSize));
  EXPECT_EQ(2u, dataSize);
}

TEST(WrapperCApiTest, ValidatesNullArguments)
{
  const std::uint8_t apdu[] = {0xc0};
  std::uint8_t output[9] = {};
  std::size_t writtenSize = 0;

  EXPECT_EQ(DLMS_WRAPPER_STATUS_INVALID_ARGUMENT,
            dlms_wrapper_encode_wpdu(0x0010u,
                                     0x0001u,
                                     apdu,
                                     sizeof(apdu),
                                     output,
                                     sizeof(output),
                                     0));

  EXPECT_EQ(DLMS_WRAPPER_STATUS_INVALID_ARGUMENT,
            dlms_wrapper_encode_wpdu(0x0010u,
                                     0x0001u,
                                     0,
                                     sizeof(apdu),
                                     output,
                                     sizeof(output),
                                     &writtenSize));
}

TEST(WrapperCApiTest, StreamDecoderCreateResetDestroy)
{
  dlms_wrapper_stream_decoder_t* decoder = 0;
  ASSERT_EQ(DLMS_WRAPPER_STATUS_OK,
            dlms_wrapper_stream_decoder_create(&decoder));
  ASSERT_NE(static_cast<dlms_wrapper_stream_decoder_t*>(0), decoder);

  dlms_wrapper_stream_decoder_reset(decoder);
  dlms_wrapper_stream_decoder_destroy(decoder);
  dlms_wrapper_stream_decoder_destroy(0);
  dlms_wrapper_stream_decoder_reset(0);
}

TEST(WrapperCApiTest, StreamDecoderCreateRejectsNullOut)
{
  EXPECT_EQ(DLMS_WRAPPER_STATUS_INVALID_ARGUMENT,
            dlms_wrapper_stream_decoder_create(0));
}

} // namespace

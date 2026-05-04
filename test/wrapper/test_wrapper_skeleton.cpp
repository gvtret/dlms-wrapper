#include "dlms/wrapper/wrapper_codec.hpp"
#include "dlms/wrapper/wrapper_ports.hpp"
#include "dlms/wrapper/wrapper_stream_decoder.hpp"

#include <gtest/gtest.h>

TEST(WrapperSkeleton, Builds)
{
  EXPECT_TRUE(dlms::wrapper::WrapperCodecSkeletonAvailable());
  EXPECT_TRUE(dlms::wrapper::WrapperStreamDecoderSkeletonAvailable());
}

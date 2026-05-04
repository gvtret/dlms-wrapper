#include "dlms/wrapper/wrapper_codec.hpp"

#include <gtest/gtest.h>

TEST(WrapperSkeleton, Builds)
{
  EXPECT_TRUE(dlms::wrapper::WrapperCodecSkeletonAvailable());
}

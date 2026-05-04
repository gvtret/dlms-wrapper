#include "dlms/wrapper/wrapper_error.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::wrapper::WrapperStatus;

TEST(WrapperStatusTest, ValuesAreStableForCApiMapping)
{
  EXPECT_EQ(0, static_cast<int>(WrapperStatus::Ok));
  EXPECT_EQ(1, static_cast<int>(WrapperStatus::NeedMoreData));
  EXPECT_EQ(2, static_cast<int>(WrapperStatus::OutputBufferTooSmall));
  EXPECT_EQ(3, static_cast<int>(WrapperStatus::InvalidArgument));
  EXPECT_EQ(4, static_cast<int>(WrapperStatus::InvalidVersion));
  EXPECT_EQ(5, static_cast<int>(WrapperStatus::InvalidLength));
  EXPECT_EQ(6, static_cast<int>(WrapperStatus::InvalidSourcePort));
  EXPECT_EQ(7, static_cast<int>(WrapperStatus::InvalidDestinationPort));
  EXPECT_EQ(8, static_cast<int>(WrapperStatus::DataTooLarge));
  EXPECT_EQ(9, static_cast<int>(WrapperStatus::FrameTooLarge));
  EXPECT_EQ(10, static_cast<int>(WrapperStatus::UnsupportedFeature));
  EXPECT_EQ(11, static_cast<int>(WrapperStatus::InternalError));
}

} // namespace

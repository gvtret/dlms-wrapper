#pragma once

namespace dlms {
namespace wrapper {

/**
 * @brief Status codes returned by the WRAPPER codec public API.
 *
 * Public/runtime API paths return these status codes instead of throwing
 * exceptions. C++ callers should use the enum names as the contract; the
 * explicit integer values keep the C ABI mapping stable.
 */
enum class WrapperStatus
{
  /// Operation completed successfully.
  Ok = 0,

  /// More input bytes are required to decode a complete WPDU.
  NeedMoreData = 1,
  /// The caller-provided output buffer is too small.
  OutputBufferTooSmall = 2,

  /// A pointer, size, or argument combination is invalid.
  InvalidArgument = 3,
  /// WRAPPER Version is not supported by this implementation.
  InvalidVersion = 4,
  /// Data length is inconsistent with the input or configured limits.
  InvalidLength = 5,
  /// Source wPort is not accepted by the current operation.
  InvalidSourcePort = 6,
  /// Destination wPort is not accepted by the current operation.
  InvalidDestinationPort = 7,

  /// DATA exceeds configured or representable limits.
  DataTooLarge = 8,
  /// Full WPDU size exceeds configured limits.
  FrameTooLarge = 9,

  /// Requested feature is outside the implemented profile.
  UnsupportedFeature = 10,

  /// Internal error or allocation failure in convenience APIs.
  InternalError = 11
};

} // namespace wrapper
} // namespace dlms

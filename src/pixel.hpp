#pragma once
#include <cstddef>
#include <cstdint>
#include <fmt/format.h>
struct Pixel {
  Pixel();
  std::uint8_t r, g, b, a = 255;
  Pixel(std::uint8_t rr, std::uint8_t gg, std::uint8_t bb);
  [[nodiscard]] std::size_t hash() const noexcept;
  bool operator==(const Pixel &) const noexcept;
};

template <> struct fmt::formatter<Pixel> {
  // Presentation format: 'f' - fixed, 'e' - exponential.

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
    // [ctx.begin(), ctx.end()) is a character range that contains a part of
    // the format string starting from the format specifications to be parsed,
    // e.g. in
    //
    //   fmt::format("{:f} - point of interest", point{1, 2});
    //
    // the range will contain "f} - point of interest". The formatter should
    // parse specifiers until '}' or the end of the range. In this example
    // the formatter should parse the 'f' specifier and return an iterator
    // pointing to '}'.

    // Parse the presentation format and store it in the formatter:
    auto it = ctx.begin(), end = ctx.end();

    // Check if reached the end of the range:
    if (it != end && *it != '}')
      throw format_error("invalid format");

    // Return an iterator past the end of the parsed range:
    return it;
  }

  // Formats the point p using the parsed format specification (presentation)
  // stored in this formatter.
  template <typename FormatContext>
  auto format(const Pixel &p, FormatContext &ctx) -> decltype(ctx.out()) {
    // ctx.out() is an output iterator to write to.
    return format_to(ctx.out(), "({}, {}, {}, {})", p.r, p.g, p.b, p.a);
  }
};

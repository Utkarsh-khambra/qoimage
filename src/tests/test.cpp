#include "../qoimage.hpp"
#include <bitset>
#include <boost/ut.hpp>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <ranges>

#include <fmt/format.h>

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
    return format_to(ctx.out(), "({}, {}, {}, {})", p.r, p.g, p.b, p.a);
  }
};

int main() {
  using namespace boost::ut;
  using namespace qoi;
  "[encode]"_test = [] {
    should("chunk rgb") = [] {
      auto p = encode({{1, 3, 4}}, 1, 1, 3);
      auto q = p | std::views::drop(14);
      expect(std::ranges::equal(q, std::vector<unsigned char>{0b10100011, 0x69,
                                                              0, 0, 0, 0, 0, 0,
                                                              0, 1}));
    };
    should("chunk range") = []() {
      auto p =
          encode(std::vector<Pixel>{{1, 3, 4}, {1, 3, 4}, {1, 3, 4}}, 3, 1, 3);
      auto q = p | std::views::drop(14);
      expect(std::ranges::equal(
          q, std::vector<unsigned char>{0b10100011, 0x69, 0b11000001, 0, 0, 0,
                                        0, 0, 0, 0, 1}));
    };
    should("chunk index") = [] {
      auto p = encode(std::vector<Pixel>{{1, 3, 4},
                                         {2, 5, 7},
                                         {3, 6, 9},
                                         {1, 3, 4},
                                         {2, 5, 7},
                                         {3, 6, 9},
                                         {3, 6, 9},
                                         {3, 6, 9},
                                         {2, 5, 7},
                                         {2, 5, 7}},
                      10, 1, 3);
      auto q = p | std::views::drop(14);
      const std::vector<unsigned char> expected_result{
          0b10100011, 0x69,       0b10100010, 0x79,       0b10100001,
          0x89,       0b00100011, 0b00000101, 0b00011011, 0b11000001,
          0b00000101, 0b11000000, 0,          0,          0,
          0,          0,          0,          0,          1};
      expect(std::ranges::equal(q, expected_result));
    };

    should("chunk difference") = [] {
      auto p = encode(std::vector<Pixel>{{1, 3, 4},
                                         {2, 4, 3},
                                         {3, 5, 2},
                                         {1, 3, 4},
                                         {2, 4, 3},
                                         {3, 5, 2},
                                         {3, 5, 2},
                                         {3, 5, 2},
                                         {2, 4, 3},
                                         {2, 4, 3}},
                      10, 1, 3);
      auto q = p | std::views::drop(14);
      const std::vector<unsigned char> expected_result{
          0b10100011, 0x69,       0b01111101, 0b01111101, 0b00100011,
          0b00100100, 0b00100101, 0b11000001, 0b00100100, 0b11000000,
          0,          0,          0,          0,          0,
          0,          0,          1};

      expect(std::ranges::equal(q, expected_result));
    };
  };

  "[decode]"_test = [] {
    should("chunk rgb") = [] {
      auto p = encode({{1, 3, 4}}, 1, 1, 3);
      auto q = decode(p);
      std::vector<Pixel> expected_result{{1, 3, 4}};
      expect(std::ranges::equal(q, expected_result));
    };
    should("chunk range") = [] {
      auto p =
          encode(std::vector<Pixel>{{1, 3, 4}, {1, 3, 4}, {1, 3, 4}}, 3, 1, 3);
      auto q = decode(p);
      expect(std::ranges::equal(
          q, std::vector<Pixel>{{1, 3, 4}, {1, 3, 4}, {1, 3, 4}}));
    };

    should("chunk index") = [] {
      auto p = encode(std::vector<Pixel>{{1, 3, 4},
                                         {2, 5, 7},
                                         {3, 6, 9},
                                         {1, 3, 4},
                                         {2, 5, 7},
                                         {3, 6, 9},
                                         {3, 6, 9},
                                         {3, 6, 9},
                                         {2, 5, 7},
                                         {2, 5, 7}},
                      10, 1, 3);
      auto q = decode(p);
      const std::vector<Pixel> expected_result{
          {1, 3, 4}, {2, 5, 7}, {3, 6, 9}, {1, 3, 4}, {2, 5, 7},
          {3, 6, 9}, {3, 6, 9}, {3, 6, 9}, {2, 5, 7}, {2, 5, 7}};
      expect(std::ranges::equal(q, expected_result));
    };

    should("chunk difference") = [] {
      auto p = encode(std::vector<Pixel>{{1, 3, 4},
                                         {2, 4, 3},
                                         {3, 5, 2},
                                         {1, 3, 4},
                                         {2, 4, 3},
                                         {3, 5, 2},
                                         {3, 5, 2},
                                         {3, 5, 2},
                                         {2, 4, 3},
                                         {2, 4, 3}},
                      10, 1, 3);
      auto q = decode(p);
      const std::vector<Pixel> expected_result{
          {1, 3, 4}, {2, 4, 3}, {3, 5, 2}, {1, 3, 4}, {2, 4, 3},
          {3, 5, 2}, {3, 5, 2}, {3, 5, 2}, {2, 4, 3}, {2, 4, 3}};
      expect(std::ranges::equal(q, expected_result));
    };
  };
}

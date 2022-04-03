#include "../qoimage.hpp"
#include <bitset>
#include <boost/ut.hpp>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <ranges>
int main() {
  using namespace boost::ut;

  "[encode]"_test = [] {
    qoi::Image i;
    should("chunk rgb") = [i]() mutable {
      auto p = i.encode({{1, 3, 4}}, 1, 1);
      auto q = p | std::views::drop(14);
      expect(std::ranges::equal(q, std::vector<unsigned char>{0b10100011, 0x69,
                                                              0, 0, 0, 0, 0, 0,
                                                              0, 1}));
    };
    should("chunk range") = [i]() mutable {
      auto p =
          i.encode(std::vector<Pixel>{{1, 3, 4}, {1, 3, 4}, {1, 3, 4}}, 3, 1);
      auto q = p | std::views::drop(14);
      expect(std::ranges::equal(
          q, std::vector<unsigned char>{0b10100011, 0x69, 0b11000001, 0, 0, 0,
                                        0, 0, 0, 0, 1}));
    };
    should("chunk index") = [i]() mutable {
      auto p = i.encode(std::vector<Pixel>{{1, 3, 4},
                                           {2, 5, 7},
                                           {3, 6, 9},
                                           {1, 3, 4},
                                           {2, 5, 7},
                                           {3, 6, 9},
                                           {3, 6, 9},
                                           {3, 6, 9},
                                           {2, 5, 7},
                                           {2, 5, 7}},
                        10, 1);
      auto q = p | std::views::drop(14);
      // auto j = 0;
      // for (auto i : q) {
      //   if (j % 2 == 0)
      //     fmt::print("{:08b}\n", i);
      //   else
      //     fmt::print("{:02x}\n", i);
      //   ++j;
      // }
      const std::vector<unsigned char> expected_result{
          0b10100011, 0x69,       0b10100010, 0x79,       0b10100001,
          0x89,       0b00100011, 0b00000101, 0b00011011, 0b11000001,
          0b00000101, 0b11000000, 0,          0,          0,
          0,          0,          0,          0,          1};
      expect(std::ranges::equal(q, expected_result));
    };

    should("chunk difference") = [i]() mutable {
      auto p = i.encode(std::vector<Pixel>{{1, 3, 4},
                                           {2, 4, 3},
                                           {3, 5, 2},
                                           {1, 3, 4},
                                           {2, 4, 3},
                                           {3, 5, 2},
                                           {3, 5, 2},
                                           {3, 5, 2},
                                           {2, 4, 3},
                                           {2, 4, 3}},
                        10, 1);
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
    qoi::Image i;
    should("chunk rgb") = [i]() mutable {
      auto p = i.encode({{1, 3, 4}}, 1, 1);
      auto q = i.decode(p);
      std::vector<Pixel> expected_result{{1, 3, 4}};
      expect(std::ranges::equal(q, expected_result));
    };
    should("chunk range") = [i]() mutable {
      auto p =
          i.encode(std::vector<Pixel>{{1, 3, 4}, {1, 3, 4}, {1, 3, 4}}, 3, 1);
      auto q = i.decode(p);
      expect(std::ranges::equal(
          q, std::vector<Pixel>{{1, 3, 4}, {1, 3, 4}, {1, 3, 4}}));
    };

    should("chunk index") = [i]() mutable {
      auto p = i.encode(std::vector<Pixel>{{1, 3, 4},
                                           {2, 5, 7},
                                           {3, 6, 9},
                                           {1, 3, 4},
                                           {2, 5, 7},
                                           {3, 6, 9},
                                           {3, 6, 9},
                                           {3, 6, 9},
                                           {2, 5, 7},
                                           {2, 5, 7}},
                        10, 1);
      auto q = i.decode(p);
      const std::vector<Pixel> expected_result{
          {1, 3, 4}, {2, 5, 7}, {3, 6, 9}, {1, 3, 4}, {2, 5, 7},
          {3, 6, 9}, {3, 6, 9}, {3, 6, 9}, {2, 5, 7}, {2, 5, 7}};
      expect(std::ranges::equal(q, expected_result));
    };

    should("chunk difference") = [i]() mutable {
      auto p = i.encode(std::vector<Pixel>{{1, 3, 4},
                                           {2, 4, 3},
                                           {3, 5, 2},
                                           {1, 3, 4},
                                           {2, 4, 3},
                                           {3, 5, 2},
                                           {3, 5, 2},
                                           {3, 5, 2},
                                           {2, 4, 3},
                                           {2, 4, 3}},
                        10, 1);
      auto q = i.decode(p);
      const std::vector<Pixel> expected_result{
          {1, 3, 4}, {2, 4, 3}, {3, 5, 2}, {1, 3, 4}, {2, 4, 3},
          {3, 5, 2}, {3, 5, 2}, {3, 5, 2}, {2, 4, 3}, {2, 4, 3}};
      expect(std::ranges::equal(q, expected_result));
    };
  };
}

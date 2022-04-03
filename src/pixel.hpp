#pragma once
#include <cstddef>
#include <cstdint>
#include <variant>
struct Pixel {
  Pixel();
  std::uint8_t r, g, b, a = 255;
  Pixel(std::uint8_t rr, std::uint8_t gg, std::uint8_t bb);
  Pixel(std::uint8_t rr, std::uint8_t gg, std::uint8_t bb, std::uint8_t aa);
  [[nodiscard]] std::size_t hash() const noexcept;
  bool operator==(const Pixel &) const noexcept;
};

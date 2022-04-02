#include "pixel.hpp"

Pixel::Pixel() : Pixel(0, 0, 0) {}
Pixel::Pixel(std::uint8_t rr, std::uint8_t gg, std::uint8_t bb)
    : r(rr), g(gg), b(bb), a(255) {}

std::size_t Pixel::hash() const noexcept {
  return (r * 3 + g * 5 + b * 7 + a * 11) % 64;
}

bool Pixel::operator==(const Pixel &o) const noexcept {
  return r == o.r && g == o.g && b == o.b && a == o.a;
}

#include "pixel.hpp"

std::size_t Pixel::hash() const noexcept {
  return (r * 3 + g * 5 + b * 7 + a * 11) % 64;
}

bool Pixel::operator==(const Pixel &o) const noexcept {
  return r == o.r && g == o.g && b == o.b && a == o.a;
}

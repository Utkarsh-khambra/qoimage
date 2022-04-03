#pragma once
#include "pixel.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>
struct qoi_header {
  std::uint32_t width;
  std::uint32_t height;
  std::uint8_t channels;
  std::uint8_t colorspace;
};

namespace qoi {
std::vector<unsigned char> encode(const std::vector<Pixel> &data, int width,
                                  int height, int channels);
std::vector<Pixel> decode(std::span<unsigned char> data);
} // namespace qoi

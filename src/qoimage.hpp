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
struct Image {
  qoi_header header;
  std::vector<Pixel> pixels;
};
std::vector<unsigned char> encode(const std::vector<Pixel> &data, int width,
                                  int height, int channels);
Image decode(std::span<unsigned char> data);
} // namespace qoi

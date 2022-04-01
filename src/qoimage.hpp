#pragma once

#include "pixel.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
struct qoi_header {
  std::uint32_t magic;
  std::uint32_t width;
  std::uint32_t height;
  std::uint8_t channels;
  std::uint8_t colorspace;
};

namespace qoi {
class Image {
public:
  Image();
  std::vector<unsigned char> encode(const std::vector<Pixel> &, int width,
                                    int height);
  void decode();

private:
  qoi_header header;
  std::vector<Pixel> pixel_data;
  std::array<Pixel, 64> previous_pixels;
};
} // namespace qoi

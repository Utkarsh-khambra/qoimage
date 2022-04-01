#include "qoimage.hpp"
#include <array>
#include <bitset>
#include <cstdio>
#include <exception>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <ranges>
#include <span>
using namespace qoi;

auto generate_header(std::uint32_t width, std::uint32_t height);
Image::Image() {
  std::fill_n(previous_pixels.begin(), previous_pixels.size(), Pixel());
}

void Image::decode() {}
static const std::array<char, 8> end_marker{0, 0, 0, 0, 0, 0, 0, 1};

auto generate_header(std::uint32_t width, std::uint32_t height) {

  return qoi_header{
      .magic = 0x716f6966,
      .width = width,
      .height = height,
      .channels = 3,  // 3 for RGB and 4 for RGBA
      .colorspace = 0 // 0 for sRGB with linear alpha
                      // 1 for all channels linear
  };
}

enum class CHUNK_TYPE { RGB, INDEX, DIFF, LUMA, RUN, UNKNOWN };

std::vector<unsigned char> Image::encode(const std::vector<Pixel> &data,
                                         int width, int height) {
  header = generate_header(static_cast<std::uint32_t>(width),
                           static_cast<std::uint32_t>(height));
  std::vector<unsigned char> output_image(sizeof(qoi_header));
  std::copy_n(reinterpret_cast<char *>(&header), output_image.size(),
              output_image.begin());
  for (std::uint32_t i = 0; i < header.height; ++i) {
    auto row = std::span(data.begin() + i * header.width, header.width);
    Pixel prev{.r = 0, .g = 0, .b = 0, .a = 255};
    int run = 0;
    for (auto &pixel : row) {
      if (pixel == prev) {
        ++run;
        prev = pixel;
        continue;
      } else if (run > 0) {
        if (run >= 63)
          std::terminate();
        std::uint8_t chunk = 0xc0 | run;
        output_image.push_back(chunk);
        run = 0;
        // No need to set prev = pixel since later stage handling current will
        // handle that as well
        // Do not continue here since we also have to handle current pixel here
      }
      if (auto pix_hash = pixel.hash(); pixel == previous_pixels[pix_hash]) {
        std::uint8_t chunk = 0x00 | pix_hash;
        output_image.push_back(chunk);
        run = 0;
        prev = pixel;
        continue;
      }
      if (auto dr = (pixel.r - prev.r + 2), dg = (pixel.g - prev.g + 2),
          db = (pixel.b - prev.b + 2);
          dr >= 0 && dr < 4 && dg >= 0 && dg < 4 && db >= 0 && db < 4) {
        std::uint8_t chunk = 0x40 | (dr << 4) | (dg << 2) | db;
        output_image.push_back(chunk);
        previous_pixels[pixel.hash()] = pixel;
        prev = pixel;
        continue;
      } else if (auto diff_green = dg + 30, dr_dg = dr - dg + 8,
                 db_dg = db - dg + 8;
                 diff_green >= 0 && diff_green <= 63 && dr_dg >= 0 &&
                 dr_dg <= 15 && db_dg >= 0 && db_dg <= 15) {
        std::uint16_t chunk =
            0x8000 | (diff_green << 8) | (dr_dg << 4) | (db_dg);
        output_image.push_back(chunk >> 8);
        output_image.push_back(chunk);
        previous_pixels[pixel.hash()] = pixel;
        prev = pixel;
        continue;
      }
      std::uint32_t chunk =
          0xfe000000 | (pixel.r << 16) | (pixel.g << 8) | pixel.b;
      output_image.push_back(chunk >> 24);
      output_image.push_back(chunk >> 16);
      output_image.push_back(chunk >> 8);
      output_image.push_back(chunk);
      previous_pixels[pixel.hash()] = pixel;
      prev = pixel;
    }
    if (run > 0) {
      if (run >= 63)
        std::terminate();
      std::uint8_t chunk = 0xc0 | run;
      output_image.push_back(chunk);
      run = 0;
    }
  }
  output_image.insert(output_image.end(), end_marker.begin(), end_marker.end());
  return output_image;
}

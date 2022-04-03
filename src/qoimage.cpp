#include "qoimage.hpp"
#include <algorithm>
#include <array>
#include <bitset>
#include <cstdio>
#include <exception>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <ranges>
#include <span>
#include <string_view>
#include <type_traits>
using namespace qoi;
using namespace std::literals;
auto generate_header(std::uint32_t width, std::uint32_t height);
Image::Image() {
  std::fill_n(previous_pixels.begin(), previous_pixels.size(), Pixel());
}

template <typename T> static void copy_int(T *data, unsigned int src) {
  auto temp_data = reinterpret_cast<unsigned char *>(data);
  for (auto i = 0; i < 4; ++i) {
    temp_data[i] = src >> (3 - i) * 8;
  }
}

template <typename T> static void copy_byte(T *data, unsigned char src) {
  auto temp_data = reinterpret_cast<unsigned char *>(data);
  *data = src;
}

qoi_header parse_header(std::span<unsigned char> data) {
  static_assert(std::is_trivially_copyable_v<qoi_header>,
                "qoi header is not trivial to copy");
  qoi_header header;
  if (std::ranges::equal(data.subspan(0, 4), "qoif"sv)) {
    copy_int(&header.width,
             static_cast<unsigned int>(data[4] << 24 | data[5] << 16 |
                                       data[6] << 8 | data[7]));
    copy_int(&header.height,
             static_cast<unsigned int>(data[8] << 24 | data[9] << 16 |
                                       data[10] << 8 | data[11]));
    copy_byte(&header.channels, data[12]);
    copy_byte(&header.colorspace, data[13]);
  }
  // if (header.colorspace > 1)
  //   std::terminate();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-overflow"
  if (header.channels - 3 > 1)
    std::terminate();
  return header;
}
#pragma GCC diagnostic pop

std::vector<Pixel> Image::decode(std::span<unsigned char> data) {
  std::fill_n(previous_pixels.begin(), previous_pixels.size(), Pixel(0, 0, 0));
  header = parse_header(data.first(14));
  data = data.last(data.size() - 14);
  std::vector<Pixel> output_data;
  Pixel prev(0, 0, 0);
  while (true) {
    if (data.front() >> 6 == 0b11) {
      if (data.front() == 0b11111110) {
        // Parse color
        data = data.last(data.size() - 1);
        static_assert(std::is_trivially_copyable_v<Pixel>,
                      "Pixel not trivial copyable");
        output_data.emplace_back(data[0], data[1], data[2]);
        data = data.last(data.size() - 3);
        previous_pixels[output_data.back().hash()] = output_data.back();
        prev = output_data.back();
        continue;
      } else {
        // parse run
        auto run = (data.front() & 0b00111111) + 1;
        data = data.last(data.size() - 1);
        if (run > 62)
          std::terminate();
        for (auto i = 0; i < run; ++i) {
          output_data.push_back(prev);
        }
        continue;
      }
    }
    if (data.front() >> 6 == 0b00 and data.front() != 0) {
      // Parse index
      std::size_t index = data.front() & 0b00111111;
      data = data.last(data.size() - 1);
      output_data.push_back(previous_pixels[index]);
      prev = output_data.back();
      continue;
    }
    if (data.front() >> 6 == 0b01) {
      // Parse Diff
      auto diff = data.front() & 0b00111111;
      data = data.last(data.size() - 1);
      unsigned char r = prev.r + (diff >> 4) - 2;
      unsigned char g = prev.g + ((diff >> 2) & 0b11) - 2;
      unsigned char b = prev.b + (diff & 0b11) - 2;
      output_data.emplace_back(r, g, b);
      previous_pixels[output_data.back().hash()] = output_data.back();
      prev = output_data.back();
      continue;
    }
    if (data.front() >> 6 == 0b10) {
      // Parse LUMA
      auto diff_green = data.front() & 0b00111111;
      auto second_byte = data[1];
      data = data.last(data.size() - 2);
      unsigned char r = (second_byte >> 4) + (diff_green - 32) + prev.r - 8;
      unsigned char g = diff_green - 32 + prev.g;
      unsigned char b = (second_byte & 0x0f) + (diff_green - 32) + prev.b - 8;
      output_data.emplace_back(r, g, b);
      previous_pixels[output_data.back().hash()] = output_data.back();
      prev = output_data.back();
      continue;
    }
    if (std::ranges::all_of(data.first(7),
                            [](auto i) noexcept -> bool { return i == 0; }) &&
        data.back() == 1) {
      break;
    }
  }
  return output_data;
}
static const std::array<char, 8> end_marker{0, 0, 0, 0, 0, 0, 0, 1};

auto generate_header(std::uint32_t width, std::uint32_t height) {
  return qoi_header{
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
  std::vector<unsigned char> output_image(14);
  copy_int(output_image.data(), 0x716f6966);
  copy_int(output_image.data() + 4, header.width);
  copy_int(output_image.data() + 8, header.height);
  copy_byte(output_image.data() + 12, header.channels);
  copy_byte(output_image.data() + 13, header.colorspace);
  Pixel prev(0, 0, 0);
  for (std::uint32_t i = 0; i < header.height; ++i) {
    auto row = std::span(data.begin() + i * header.width, header.width);
    int run = 0;
    for (auto &pixel : row) {
      if (pixel == prev) {
        ++run;
        if (run == 62) {
          // Push the current run and then put an index
          std::uint8_t chunk = 0xc0 | (run - 1);
          output_image.push_back(chunk);
          run = 0;
        }
      } else {
        if (run > 0) {
          std::uint8_t chunk = 0xc0 | (run - 1);
          output_image.push_back(chunk);
          run = 0;
          // No need to set prev = pixel since later stage handling current will
          // handle that as well
          // Do not continue here since we also have to handle current pixel
          // here
        }
        if (auto pix_hash = pixel.hash(); pixel == previous_pixels[pix_hash]) {
          std::uint8_t chunk = 0x00 | pix_hash;
          output_image.push_back(chunk);

        } else {
          previous_pixels[pixel.hash()] = pixel;
          if (auto dr = (pixel.r - prev.r + 2), dg = (pixel.g - prev.g + 2),
              db = (pixel.b - prev.b + 2);
              dr >= 0 && dr < 4 && dg >= 0 && dg < 4 && db >= 0 && db < 4) {
            std::uint8_t chunk = 0x40 | (dr << 4) | (dg << 2) | db;
            output_image.push_back(chunk);
          } else if (auto diff_green = dg + 30, dr_dg = dr - dg + 8,
                     db_dg = db - dg + 8;
                     diff_green >= 0 && diff_green <= 63 && dr_dg >= 0 &&
                     dr_dg <= 15 && db_dg >= 0 && db_dg <= 15) {
            std::uint16_t chunk =
                0x8000 | (diff_green << 8) | (dr_dg << 4) | (db_dg);
            output_image.push_back(chunk >> 8);
            output_image.push_back(chunk);
          } else {
            std::uint32_t chunk =
                0xfe000000 | (pixel.r << 16) | (pixel.g << 8) | pixel.b;
            output_image.push_back(chunk >> 24);
            output_image.push_back(chunk >> 16);
            output_image.push_back(chunk >> 8);
            output_image.push_back(chunk);
          }
        }
      }
      prev = pixel;
    }
    if (run > 0) {
      if (run >= 63) {
        std::terminate();
      }
      std::uint8_t chunk = (0xc0 | run) - 1;
      output_image.push_back(chunk);
      run = 0;
    }
  }
  output_image.insert(output_image.end(), end_marker.begin(), end_marker.end());
  return output_image;
}

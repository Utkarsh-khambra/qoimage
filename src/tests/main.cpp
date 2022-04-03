#define STB_IMAGE_IMPLEMENTATION
#include "../qoimage.hpp"
#include "stb.h"
#include <bit>
#include <fstream>
int main() {
  int x, y, n;
  auto pixel_array =
      stbi_load("/home/utkarsh/qoimage/src/test_files/dice.png", &x, &y, &n, 0);
  std::vector<Pixel> pixels;
  for (auto k = 0; k < y * x * n; k += n) {
    if (n == 3)
      pixels.emplace_back(pixel_array[k], pixel_array[k + 1],
                          pixel_array[k + 2]);
    else
      pixels.emplace_back(pixel_array[k], pixel_array[k + 1],
                          pixel_array[k + 2], pixel_array[k + 3]);
  }
  stbi_image_free(pixel_array);
  auto q = qoi::encode(pixels, x, y, n);
  std::ofstream f("test.qoi", std::ios_base::binary);
  f.write(reinterpret_cast<char *>(q.data()), q.size());
}

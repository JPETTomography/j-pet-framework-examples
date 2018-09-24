#pragma once

#include <cstdint>
#include <limits>
#include <algorithm>

namespace util {

/// Writes custom image data to \a PNG file
class png_writer {
 public:
  /// Constructs new \a PNG writer writing to given path
  png_writer(std::string fn, double scale_max = 0);

  /// Writes \a PNG file header defining image dimensions
  template <typename byte_type = uint8_t>
  void write_header(unsigned int width, unsigned int height) {
    // only 8bpp & 16bpp are supported by PNG
    priv_write_header(width, height, sizeof(byte_type) > 1 ? 16 : 8);
  }

  /// Writes image row
  ////
  /// Provided array should match dimensions provided by \ref write_header
  template <typename byte_type = uint8_t> void write_row(byte_type* row) {
    priv_write_row(reinterpret_cast<unsigned char*>(row));
  }

  ~png_writer();

  template <typename T>
  void write(unsigned int width, unsigned int height, const T* output) {
    write_header(width, height);

    T output_max = scale_max;
    if (output_max == 0) {
      for (unsigned int i = 0; i < width * height; ++i) {
        output_max = std::max(output_max, output[i]);
      }
    }

    auto output_gain =
        static_cast<double>(std::numeric_limits<uint8_t>::max()) / output_max;

    uint8_t* row = (uint8_t*)alloca(width);
    for (unsigned int y = 0; y < height; ++y) {
      for (unsigned int x = 0; x < width; ++x) {
        row[x] = std::max(0.0,
                          std::numeric_limits<uint8_t>::max() -
                              output_gain * output[y * width + x]);
      }
      write_row(row);
    }
  }

 private:
  void priv_write_header(unsigned int width,
                         unsigned int height,
                         unsigned int bpp);
  void priv_write_row(unsigned char* row);
  struct png_writer_private* priv;
  double scale_max;
};
}  // util

#include <string>
#include <ostream>

#ifdef HAVE_LIBPNG
#include <png.h>
struct png_writer_private {
  FILE* fp;
  png_structp png_ptr;
  png_infop info_ptr;
};
#endif

#include "png_writer.h"

namespace util {

png_writer::png_writer(std::string fn, double scale_max)
    : scale_max(scale_max) {
#ifdef HAVE_LIBPNG
  priv = new png_writer_private;
  priv->fp = NULL;
  priv->png_ptr = NULL;
  priv->info_ptr = NULL;

  if (!(priv->png_ptr =
            png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
    throw(std::string("cannot create png version"));
  }

  if (!(priv->info_ptr = png_create_info_struct(priv->png_ptr))) {
    throw(std::string("cannot create png info"));
  }

  if (setjmp(png_jmpbuf(priv->png_ptr))) {
    throw(std::string("cannot hook png exception"));
  }

  if (!(priv->fp = fopen(fn.c_str(), "wb"))) {
    throw(std::string("cannot create output file"));
  }

  png_init_io(priv->png_ptr, priv->fp);
#else
  (void)fn;  // unused
  throw("PNG support is not available in this build");
#endif
}

void png_writer::priv_write_header(unsigned int width,
                                   unsigned int height,
                                   unsigned int bpp) {
#ifdef HAVE_LIBPNG
  png_set_IHDR(priv->png_ptr,
               priv->info_ptr,
               width,
               height,
               bpp,
               PNG_COLOR_TYPE_GRAY,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE,
               PNG_FILTER_TYPE_BASE);
  png_write_info(priv->png_ptr, priv->info_ptr);
#else
  (void)width, (void)height, (void)bpp;  // unused
#endif
}

void png_writer::priv_write_row(unsigned char* row) {
#ifdef HAVE_LIBPNG
  png_write_row(priv->png_ptr, row);
#else
  (void)row;                             // unused
#endif
}

png_writer::~png_writer() {
#ifdef HAVE_LIBPNG
  if (priv->fp) {
    png_write_end(priv->png_ptr, NULL);
    fclose(priv->fp);
  }
  if (priv->info_ptr)
    png_free_data(priv->png_ptr, priv->info_ptr, PNG_FREE_ALL, -1);
  if (priv->png_ptr)
    png_destroy_write_struct(&priv->png_ptr, (png_infopp)NULL);
  if (priv)
    delete priv;
#endif
}

}  // util

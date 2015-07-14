#ifndef IMTOOLBOX_IMAGE_H
#define IMTOOLBOX_IMAGE_H
#include <cmath>
#include <cstring>
#include <cstdio>
#include <jpeglib.h>
#include <setjmp.h>
#include <png.h>

namespace imtoolbox {

struct my_error_mgr {
  struct jpeg_error_mgr pub; /* "public" fields */
  jmp_buf setjmp_buffer;     /* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

template <typename T>
inline void imread_jpeg_rows(T *pix, struct jpeg_decompress_struct &cinfo,
                             size_t row_stride) noexcept {
  // jpeg_read_scanlines expects an array of pointers to scanlines.
  // Here we create an array of one element long
  JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo,
                                                 JPOOL_IMAGE, row_stride, 1);
  uint8_t *first{buffer[0]};
  // Scan line by line
  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, buffer, 1);
    std::copy(first, first + row_stride, pix);
    pix += row_stride;
  }
}

template <>
inline void imread_jpeg_rows<uint8_t>(uint8_t *pix,
                                      struct jpeg_decompress_struct &cinfo,
                                      size_t row_stride) noexcept {
  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, &pix, 1);
    pix += row_stride;
  }
}

// template version of imread_jpeg
template <typename T, size_t N>
matrix<T, N> imread_jpeg(const char *file_name) noexcept {
  static_assert(N == 2 || N == 3, "Return matrix order can only be 2 or 3");

  FILE *fp{fopen(file_name, "rb")}; // Open file
  if (fp == nullptr) {
    println_e("File not found");
    return matrix<T, N>{};
  }

  struct jpeg_decompress_struct cinfo; // JPEG decompression parameters
  struct my_error_mgr jerr;            // Private extension JPEG error handler
  cinfo.err = jpeg_std_error(&jerr.pub);

  // Install custom error handler
  jerr.pub.error_exit = [](j_common_ptr cinfo) {
    char jmsg[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, jmsg);
    println_e(jmsg);
    my_error_ptr myerr =
        (my_error_ptr)
            cinfo->err; // Hack! err points to the beginning of my_error_ptr
    longjmp(myerr->setjmp_buffer, 1);
  };

  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);
    return matrix<T, N>{};
  }

  jpeg_create_decompress(&cinfo); // Initialize the JPEG decompression object

  jpeg_stdio_src(&cinfo, fp); // Specify data source

  jpeg_read_header(&cinfo, TRUE); // Read file parameters

  jpeg_start_decompress(&cinfo); // Start decompressor

  if (N == 2 && cinfo.output_components > 1) {
    println_e("Not a gray scale image");
    longjmp(jerr.setjmp_buffer, 1);
  }

  if (N == 3 && cinfo.output_components == 1) {
    println_w("Gray scale image is loaded as 3 dimensions");
  }

  matrix3<T> mat(cinfo.output_height, cinfo.output_width,
                 cinfo.output_components); // Return result

  auto row_stride = cinfo.output_width * cinfo.output_components;

  imread_jpeg_rows<T>(mat.data(), cinfo, row_stride);

  jpeg_finish_decompress(&cinfo); // Finish decompression

  jpeg_destroy_decompress(&cinfo);
  fclose(fp);
  return resize<N>(mat);
}

template <typename T>
inline void imread_png_rows(T *pix, const png_structp png_ptr, size_t height,
                            size_t row_stride) noexcept {
  std::vector<uint8_t> buffer(row_stride);
  auto p = buffer.data();

  for (uint32_t i = 0; i < height; ++i) {
    png_read_rows(png_ptr, &p, nullptr, 1);
    // Copy to mat
    std::copy(p, p + row_stride, pix);
    pix += row_stride;
  }
}

template <>
inline void imread_png_rows<uint8_t>(uint8_t *pix, const png_structp png_ptr,
                                     size_t height,
                                     size_t row_stride) noexcept {
  for (uint32_t i = 0; i < height; ++i) {
    png_read_rows(png_ptr, &pix, nullptr, 1);
    pix += row_stride;
  }
}

template <typename T, size_t N>
matrix<T, N> imread_png(const char *file_name) noexcept {
  static_assert(N == 2 || N == 3, "Return matrix order can only be 2 or 3");

  FILE *fp{fopen(file_name, "rb")};
  if (fp == nullptr) {
    println_e("File not found");
    return matrix<T, N>{};
  }

  png_structp png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr,
                             nullptr); // Create and initialize the png_struct
  if (png_ptr == nullptr) {
    fclose(fp);
    return matrix<T, N>{};
  }

  png_infop info_ptr = png_create_info_struct(
      png_ptr); // Allocate and initialize the memory for image information
  if (info_ptr == nullptr) {
    /* Free all of the memory associated with the png_ptr and info_ptr */
    png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    fclose(fp);
    return matrix<T, N>{};
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    /* Free all of the memory associated with the png_ptr and info_ptr */
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(fp);
    /* If we get here, we had a problem reading the file */
    return matrix<T, N>{};
  }

  /* Set up the input control if you are using standard C streams */
  png_init_io(png_ptr, fp);

  png_read_info(png_ptr, info_ptr); // Get all of the information from PNG file
                                    // before the first image data chunk (IDAT)

  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
               &interlace_type, nullptr, nullptr);

  if (bit_depth != 8) {
    println_e("Not support bit depth different than 8 for now");
    longjmp(png_jmpbuf(png_ptr), 1);
  }

  if (interlace_type != PNG_INTERLACE_NONE) {
    println_e("Not support interlace for now");
    longjmp(png_jmpbuf(png_ptr), 1);
  }

  uint32_t depth = 0;
  switch (color_type) {
  case PNG_COLOR_TYPE_GRAY:
    depth = 1;
    break;
  case PNG_COLOR_TYPE_RGB:
    depth = 3;
    break;
  default:
    println_e("Color type is not supported");
    longjmp(png_jmpbuf(png_ptr), 1);
  }

  if (N == 2 && depth > 1) {
    println_e("Not a gray scale image");
    longjmp(png_jmpbuf(png_ptr), 1);
  }

  if (N == 3 && depth == 1) {
    println_w("Gray scale image is loaded as 3 dimensions");
  }

  uint32_t row_stride = width * depth;

  matrix3<T> mat(height, width, depth);

  imread_png_rows<T>(mat.data(), png_ptr, height, row_stride);

  // Read rest of file, and get additional chunks in info_ptr
  png_read_end(png_ptr, info_ptr);

  // Clean up after the read, and free any memory allocated
  png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

  // Close the file
  fclose(fp);
  return resize<N>(mat);
}

template <typename M>
int imwrite_jpeg(const M &mat, const char *file_name,
                 uint quality = 100) noexcept {
  static_assert(is_matrix<M>() && (M::order == 2 || M::order == 3),
                "Param should be a matrix of order 2 or 3");

  FILE *fp{fopen(file_name, "wb")}; // Open file
  if (fp == nullptr) {
    println_e("File not found");
    return -1;
  }

  struct jpeg_compress_struct cinfo; // JPEG decompression parameters
  struct my_error_mgr jerr;          // Private extension JPEG error handler
  cinfo.err = jpeg_std_error(&jerr.pub);

  // Install custom error handler
  jerr.pub.error_exit = [](j_common_ptr cinfo) {
    char jmsg[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, jmsg);
    println_e(jmsg);
    my_error_ptr myerr = (my_error_ptr)cinfo->err;
    longjmp(myerr->setjmp_buffer, 1);
  };
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_compress(&cinfo);
    fclose(fp);
    return -1;
  }

  jpeg_create_compress(&cinfo); // Initialize JPEG compression object

  jpeg_stdio_dest(&cinfo, fp); // Specify data destination

  cinfo.image_height = mat.size(0); // Specify image height
  cinfo.image_width = mat.size(1);  // Specify image width

  // Specify image components and color space
  if (M::order == 2 || (M::order == 3 && mat.size(2) == 1)) {
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;
  } else if (M::order == 3 && mat.size(2) == 3) {
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
  } else {
    println_e("Invalid components or color space");
    longjmp(jerr.setjmp_buffer, 1);
  }

  jpeg_set_defaults(&cinfo);

  jpeg_set_quality(&cinfo, quality, true);

  jpeg_start_compress(&cinfo, true);

  auto row_stride = cinfo.image_width * cinfo.input_components;

  auto pix = mat.begin();

  std::vector<uint8_t> buffer(row_stride);
  auto p = buffer.data();

  while (cinfo.next_scanline < cinfo.image_height) {
    // Round and copy data to buffer
    for (size_t i = 0; i < row_stride; ++i) {
      buffer[i] = std::round(*pix);
      ++pix;
    }
    jpeg_write_scanlines(&cinfo, &p, 1);
  }

  jpeg_finish_compress(&cinfo);

  jpeg_destroy_compress(&cinfo);

  fclose(fp);
  return 0;
}

template <typename M>
int imwrite_png(const M &mat, const char *file_name) noexcept {
  static_assert(is_matrix<M>() && (M::order == 2 || M::order == 3),
                "Param should be a matrix of order 2 or 3");

  FILE *fp{fopen(file_name, "wb")};
  if (fp == nullptr) {
    println_e("File not found");
    return -1;
  }

  // Create and initialize the png_struct with the desired error handler
  // functions.
  png_structp png_ptr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  if (png_ptr == nullptr) {
    fclose(fp);
    return -1;
  }

  // Allocate/initialize the image information data.
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == nullptr) {
    png_destroy_write_struct(&png_ptr, nullptr);
    fclose(fp);
    return -1;
  }

  // Set error handling
  if (setjmp(png_jmpbuf(png_ptr))) {
    // If we get here, we had a problem writing the file
    fclose(fp);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return -1;
  }

  // Set up the output control if you are using standard C streams
  png_init_io(png_ptr, fp);

  png_uint_32 height = mat.size(0), width = mat.size(1);

  if (height > PNG_UINT_32_MAX / (sizeof(png_bytep))) {
    png_error(png_ptr, "Image is too tall to process in memory");
  }

  int color_type = -1;
  size_t depth = 0;
  if (M::order == 2 || (M::order == 3 && mat.size(2) == 1)) {
    color_type = PNG_COLOR_TYPE_GRAY;
    depth = 1;
  } else if (M::order == 3 && mat.size(2) == 3) {
    color_type = PNG_COLOR_TYPE_RGB;
    depth = 3;
  } else {
    println_e("Invalid color type");
    longjmp(png_jmpbuf(png_ptr), 1);
  }

  png_set_IHDR(png_ptr, info_ptr, width, height, 8, color_type,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
               PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);

  auto row_stride = width * depth;
  auto pix = mat.begin();

  std::vector<uint8_t> buffer(row_stride);
  uint8_t *p = buffer.data();

  for (uint32_t i = 0; i < height; ++i) {
    // Round and copy data to buffer
    for (size_t j = 0; j < row_stride; ++j) {
      buffer[j] = std::round(*pix);
      ++pix;
    }
    png_write_rows(png_ptr, &p, 1);
  }

  png_write_end(png_ptr, info_ptr);

  // Clean up after the write, and free any memory allocated
  png_destroy_write_struct(&png_ptr, &info_ptr);

  // Close the file
  fclose(fp);
  return 0;
}

inline bool is_sig_equal(const uint8_t *sig1, const uint8_t *sig2,
                         size_t num_to_check) noexcept {
  return (memcmp(sig1, sig2, num_to_check) == 0);
}

inline bool is_png(const uint8_t *sig) noexcept {
  constexpr int png_bytes_to_check = 8;
  constexpr uint8_t png_sig[png_bytes_to_check] = {137, 80, 78, 71,
                                                   13,  10, 26, 10};
  return is_sig_equal(sig, png_sig, png_bytes_to_check);
}

inline bool is_jpeg(const uint8_t *sig) noexcept {
  constexpr int jpeg_bytes_to_check = 4;
  constexpr int num_jpeg_sigs = 3;
  constexpr uint8_t jpeg_sig[num_jpeg_sigs][jpeg_bytes_to_check] = {
      {255, 216, 255, 224}, {255, 216, 255, 225}, {255, 216, 255, 232}};

  for (int i = 0; i < num_jpeg_sigs; ++i) {
    if (is_sig_equal(sig, jpeg_sig[i], jpeg_bytes_to_check)) {
      return true;
    }
  }
  return false;
}

template <typename T, size_t N = 3>
inline matrix<T, N> imread(const char *file_name) noexcept {
  static_assert(N == 2 || N == 3, "Return matrix order can only be 2 or 3");
  FILE *fp{fopen(file_name, "rb")};
  if (fp == nullptr) {
    println_e("File not found");
    return matrix<T, N>{};
  }

  constexpr int max_bytes = 8;
  uint8_t sig[max_bytes];
  if (fread(sig, 1, max_bytes, fp) != max_bytes) {
    fclose(fp);
    return matrix<T, N>{};
  }

  fclose(fp);

  matrix<T, N> ret{};
  if (is_png(sig)) {
    ret = imread_png<T, N>(file_name);
  } else if (is_jpeg(sig)) {
    ret = imread_jpeg<T, N>(file_name);
  } else {
    println_e("File type isn't supported");
  }
  return ret;
}

template <typename T, size_t N = 3>
inline matrix<T, N> imread(const std::string &file_name) noexcept {
  static_assert(N == 2 || N == 3, "Return matrix order can only be 2 or 3");
  return imread<T, N>(file_name.c_str());
}

template <typename M>
inline int imwrite(const M &mat, const char *file_name) noexcept {
  static_assert(is_matrix<M>() && (M::order == 2 || M::order == 3),
                "Param should be a matrix of order 2 or 3");

  const char *pch = strchr(file_name, '.');
  const char *last_pch = nullptr;
  while (pch != nullptr) {
    last_pch = pch;
    pch = strchr(pch + 1, '.');
  }

  if (last_pch == nullptr) {
    println_e("No extension supplied");
    return -1;
  }

  last_pch += 1;

  int ret_val = 0;
  if (strncmp(last_pch, "png", 3) == 0) {
    ret_val = imwrite_png(mat, file_name);
  } else if (strncmp(last_pch, "jpg", 3) == 0 ||
             strncmp(last_pch, "jpeg", 4) == 0) {
    ret_val = imwrite_jpeg(mat, file_name);
  } else {
    println_e("Unknown extension");
    ret_val = -1;
  }
  return ret_val;
}

template <typename M>
inline int imwrite(const M &mat, const std::string &file_name) noexcept {
  static_assert(is_matrix<M>() && (M::order == 2 || M::order == 3),
                "Param should be a matrix of order 2 or 3");
  return imwrite(mat, file_name.c_str());
}
} // namespace imtoolbox
#endif // IMTOOLBOX_IMAGE_H

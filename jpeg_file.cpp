// Copyright (c) 2021-2022 Luis Peñaranda. All rights reserved.
//
// This file is part of empdfer.
//
// Empdfer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Empdfer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with empdfer.  If not, see <http://www.gnu.org/licenses/>.

#include "jpeg_file.h"
#include "matrix.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <jerror.h>

void empdfer::create_jpeg(const std::string& compressed_file,
                          unsigned char* data, long width, long height,
                          unsigned components, J_COLOR_SPACE color_space,
                          int quality)
{
  jpeg_compress_struct cinfo;
  struct jpeg_error_mgr err;

  FILE* outfile;
  if ((outfile = fopen(compressed_file.c_str(), "wb")) == NULL)
  {
    std::cerr << "can't open output file \"" << compressed_file << "\"" << std::endl;
    exit(-1);
  }

  cinfo.err = jpeg_std_error(&err);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = components;
  cinfo.in_color_space = color_space;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE);

  jpeg_start_compress(&cinfo, TRUE);

  int row_stride = width * components;
  JSAMPROW row_pointer[1];

  while (cinfo.next_scanline < cinfo.image_height)
  {
    row_pointer[0] = &data[cinfo.next_scanline * row_stride];
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);
  fclose(outfile);
  jpeg_destroy_compress(&cinfo);
}

void empdfer::recompress_jpeg(const std::string& input_file,
                              const std::string& compressed_file, int quality)
{
  jpeg_decompress_struct dinfo;
  struct jpeg_error_mgr err;

  jpeg_create_decompress(&dinfo);
  dinfo.err = jpeg_std_error(&err);

  FILE* infile;
  if ((infile = fopen(input_file.c_str(), "rb")) == NULL)
  {
    std::cerr << "can't open input file \"" << input_file << "\"" << std::endl;
    exit(-1);
  }

  jpeg_stdio_src(&dinfo, infile);
  jpeg_read_header(&dinfo, (boolean)0);
  
  jpeg_start_decompress(&dinfo);

  int row_stride = dinfo.output_width * dinfo.output_components;
  JSAMPARRAY buffer = (*dinfo.mem->alloc_sarray)((j_common_ptr) &dinfo, JPOOL_IMAGE, row_stride, 1);
  unsigned char* uncompressed_image = (unsigned char*)malloc(sizeof(unsigned char) * row_stride * dinfo.output_height);

  while (dinfo.output_scanline < dinfo.output_height) {
    buffer[0] = uncompressed_image + dinfo.output_scanline * row_stride;
    jpeg_read_scanlines(&dinfo, buffer, 1);
  }

  jpeg_finish_decompress(&dinfo);
  fclose(infile);

  create_jpeg(compressed_file, uncompressed_image, dinfo.image_width,
              dinfo.image_height, dinfo.num_components, dinfo.out_color_space,
              quality);

  jpeg_destroy_decompress(&dinfo);
}

paddlefish::PagePtr empdfer::jpeg_page(const std::string& input_file,
                                       double page_x_mm, double page_y_mm,
                                       double img_x_mm, double img_y_mm,
                                       int quality, double rotation,
                                       bool shrink)
{
  paddlefish::PagePtr p(new paddlefish::Page());

  // Compute image size using libjpeg.
  jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr err;

  jpeg_create_decompress(&cinfo);
  cinfo.err = jpeg_std_error(&err);

  FILE* infile;
  if ((infile = fopen(input_file.c_str(), "rb")) == NULL)
  {
    std::cerr << "can't open input file \"" << input_file << "\"" << std::endl;

    exit(-1);
  }

  jpeg_stdio_src(&cinfo, infile);
  jpeg_read_header(&cinfo, (boolean)0);
  fclose(infile);

  // Done with libjpeg.

  // Compute the image size based on the resolution in the jpg.
  if (img_x_mm == -1. && img_y_mm == -1.)
  {
    // Assume density is specified in DPI. Convert it to dots per mm.
    double x_density_dpmm = (double)cinfo.X_density / 25.4;
    double y_density_dpmm = (double)cinfo.Y_density / 25.4;

    img_x_mm = cinfo.image_width / x_density_dpmm;
    img_y_mm = cinfo.image_height / y_density_dpmm;
  }
  // Compute the missing dimensions to maintain aspect ratio in case one size
  // was not specified.
  else if (img_x_mm == -1.)
    img_x_mm = (double)cinfo.image_width * img_y_mm / cinfo.image_height;
  else
    img_y_mm = (double)cinfo.image_height * img_x_mm / cinfo.image_width;

  // Set the page size.
  p->set_mediabox(0, 0, MILIMETERS(page_x_mm), MILIMETERS(page_y_mm));

  // Compute the PDF transformation matrix for the image.
  double matrix23[6];
  empdfer::fill_matrix(matrix23, img_x_mm, img_y_mm, page_x_mm, page_y_mm,
                       rotation, shrink);

  if (quality == -1)
  {
    // Add the jpeg image to the page. For this, try find which color space
    // the image is in.
      p->add_jpeg_image(input_file,
                        cinfo.image_width, cinfo.image_height,
                        matrix23,
                        cinfo.jpeg_color_space == JCS_GRAYSCALE ?
                        COLORSPACE_DEVICEGRAY : COLORSPACE_DEVICERGB);
  }
  else
  {
    std::string compressed_file =
      std::filesystem::temp_directory_path().string() + "/" + input_file +
      "_compressed_" + std::to_string(quality);

    empdfer::recompress_jpeg(input_file, compressed_file, quality);

    // Add the recompressed image.
    p->add_jpeg_image(compressed_file, cinfo.image_width, cinfo.image_height,
                      matrix23,
                      cinfo.jpeg_color_space == JCS_GRAYSCALE ?
                      COLORSPACE_DEVICEGRAY : COLORSPACE_DEVICERGB);
  }

  return p;
}

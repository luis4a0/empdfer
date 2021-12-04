// Copyright (c) 2021 Luis Peñaranda. All rights reserved.
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

#include "jpeg.h"

#include <iostream>
#include <jerror.h>
#include <jpeglib.h>

void create_jpeg(const std::string& compressed_file, unsigned char* data,
                 long width, long height, unsigned components,
                 J_COLOR_SPACE color_space, int quality)
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
  jpeg_read_header(&dinfo, 0);
  
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

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

#include "create_page.h"
#include "jpeg.h"

#include <cstring>
#include <iostream>
#include <fstream>
#include <jerror.h>
#include <jpeglib.h>
#include <sstream>

paddlefish::PagePtr empdfer::create_page(const std::string& input_file,
                                         double page_x_mm, double page_y_mm,
                                         double img_x_mm, double img_y_mm,
                                         int quality)
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
  jpeg_read_header(&cinfo, 0);
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

  // Compute the margins needed to center the image on page.
  double margin_x_mm = (page_x_mm - img_x_mm) / 2.;
  double margin_y_mm = (page_y_mm - img_y_mm) / 2.;

  // Set the page size.
  p->set_mediabox(0, 0, MILIMETERS(page_x_mm), MILIMETERS(page_y_mm));

  // TODO: remove showing this debug information.
  std::cerr << "jpeg_color_space=" << cinfo.jpeg_color_space <<
    ", out_color_space=" << cinfo.out_color_space << std::endl;

  if (quality == -1)
  {
    // Add the jpeg image to the page. For this, try find which color space
    // the image is in.
      p->add_jpeg_image(input_file,
                        cinfo.image_width, cinfo.image_height,
                        MILIMETERS(margin_x_mm), MILIMETERS(margin_y_mm),
                        MILIMETERS(img_x_mm), MILIMETERS(img_y_mm),
                        cinfo.jpeg_color_space == JCS_GRAYSCALE ?
                        COLORSPACE_DEVICEGRAY : COLORSPACE_DEVICERGB);
  }
  else
  {
    std::string compressed_file = input_file + "_compressed_" + std::to_string(quality);

    empdfer::recompress_jpeg(input_file, compressed_file, quality);

    // Add the resulting image
    p->add_jpeg_image(compressed_file, cinfo.image_width, cinfo.image_height,
                      MILIMETERS(margin_x_mm), MILIMETERS(margin_y_mm),
                      MILIMETERS(img_x_mm), MILIMETERS(img_y_mm),
                      cinfo.jpeg_color_space == JCS_GRAYSCALE ?
                      COLORSPACE_DEVICEGRAY : COLORSPACE_DEVICERGB);

    // TODO: Remember to remove the file after the PDF is created!
  }

  return p;
}

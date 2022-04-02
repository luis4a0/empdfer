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

#include "png_file.h"

#include <png.h>

#include <cmath>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>

// See http://www.libpng.org/pub/png/libpng-1.2.5-manual.html#section-3 for
// explanation on how to use libpng.
paddlefish::PagePtr empdfer::png_page(const std::string& input_file,
                                      double page_x_mm, double page_y_mm,
                                      double img_x_mm, double img_y_mm,
                                      int quality, bool shrink)
{
    paddlefish::PagePtr p(new paddlefish::Page());

    unsigned x_size, y_size;

    FILE *fp = fopen(input_file.c_str(), "rb");

    if (!fp)
        throw std::runtime_error(input_file + ": unable to open file");

    // Read the header of the file.
    unsigned char header[8];
    png_size_t number_to_check = 8;
    if (fread(header, 1, 8, fp) != (unsigned long)ftell(fp))
        throw std::runtime_error(input_file + ": could not read PNG header");

    // Check the file is valid.
    if(png_sig_cmp(header, 0, number_to_check))
        throw std::runtime_error(input_file + ": invalid PNG file");

    // Initialize.
    png_structp png_ptr =
        png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        throw std::runtime_error(input_file + ": cannot init PNG structure");

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr){
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        throw std::runtime_error(input_file + ": cannot init PNG info struct");
    }

    if (setjmp(png_jmpbuf(png_ptr)))
        throw std::runtime_error(input_file + ": cannot set jump to struct");

    // Read file header and the information we need.

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, number_to_check);
    png_read_info(png_ptr, info_ptr);

    x_size = png_get_image_width(png_ptr, info_ptr);
    y_size = png_get_image_height(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    png_byte channels = png_get_channels(png_ptr, info_ptr);

    // Get color type, and convert palette to RGB if needed.
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(png_ptr);
        color_type = PNG_COLOR_TYPE_RGB;
        channels = 3;
        bit_depth = 8;
    }

    /*if (color_type & PNG_COLOR_MASK_ALPHA)
    {
        // TODO: combine the alpha with the RGB or gray bytes.
        std::cout << "removing alpha" << std::endl;

        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        {
            std::cout << "valid" << std::endl;
            png_set_tRNS_to_alpha(png_ptr);
        }

        png_set_strip_alpha(png_ptr);
        color_type -= PNG_COLOR_MASK_ALPHA;
        channels = color_type == PNG_COLOR_TYPE_RGB ? 3 : 1;
    }*/

    // Get the PNG resolution.
    unsigned res_x, res_y;
    int unit_type;
    png_get_pHYs(png_ptr, info_ptr, &res_x, &res_y, &unit_type);

    // Done reading data from the header.

    // The resolution is usually specified in dots per meter, we need it
    // in dots per millimeter.
    double x_density_dpmm, y_density_dpmm;
    switch (unit_type)
    {
        case PNG_RESOLUTION_METER:
            x_density_dpmm = res_x / 1000.;
            y_density_dpmm = res_y / 1000.;
            break;
        default:
            // If the resolution cannot be determined, set it to 300dpi.
            x_density_dpmm = y_density_dpmm = 300. / 25.4;
            break;
    }

    // Compute the image size based on the resolution in the PNG.
    if (img_x_mm == -1. && img_y_mm == -1.)
    {
        img_x_mm = x_size / x_density_dpmm;
        img_y_mm = y_size / y_density_dpmm;
    }

    // Compute the missing dimensions to maintain aspect ratio in case one size
    // was not specified.
    else if (img_x_mm == -1.)
        img_x_mm = (double)x_size * img_y_mm / y_size;
    else
        img_y_mm = (double)y_size * img_x_mm / x_size;

    // Shrink to fit page if needed.
    if (shrink)
    {
        if (img_x_mm > page_x_mm)
        {
            img_y_mm *= page_x_mm / img_x_mm;
            img_x_mm = page_x_mm;
        }

        if (img_y_mm > page_y_mm)
        {
            img_x_mm *= page_y_mm / img_y_mm;
            img_y_mm = page_y_mm;
        }
    }

    // Compute the margins needed to center the image on page.
    double margin_x_mm = (page_x_mm - img_x_mm) / 2.;
    double margin_y_mm = (page_y_mm - img_y_mm) / 2.;

    unsigned char *image = (unsigned char*)png_malloc(
        png_ptr, y_size * x_size * bit_depth * channels * sizeof(png_bytep));
    unsigned char *mask = NULL;

    png_bytep* row_pointers = (png_bytep*)malloc(y_size * sizeof(png_bytep));

    for (unsigned i = 0; i < y_size; ++i)
        row_pointers[i] =
            image + i * x_size * channels * bit_depth / sizeof(png_bytep);

    png_read_image(png_ptr, row_pointers);

    // If the image has transparency, separate the actual colors from the mask.
    if (color_type & PNG_COLOR_MASK_ALPHA)
    {
        unsigned color_channels = channels - 1;

        unsigned char *plain = (unsigned char*)malloc(
                y_size * x_size * bit_depth * color_channels * sizeof(png_bytep));

        mask = (unsigned char*)malloc(
                y_size * x_size * bit_depth * sizeof(unsigned char));

        for (unsigned row = 0; row < y_size; ++row)
            for (unsigned col = 0; col < x_size; ++col)
            {
                    unsigned offset = row * x_size + col;
                    for (unsigned comp = 0; comp < color_channels; ++comp)
                        plain[color_channels * offset + comp] =
                            image[channels * offset + comp];
                    mask[offset] = image[channels * offset + 3];
            }

        // Swap contents of plain and image.
        unsigned char *temp = image;
        image = plain;
        plain = temp;

        color_type -= PNG_COLOR_MASK_ALPHA;
        channels = color_channels;
    }

    //png_read_end(NULL, NULL);
    //png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    fclose(fp);

    // Set the page size.
    p->set_mediabox(0, 0, MILIMETERS(page_x_mm), MILIMETERS(page_y_mm));

    /*
    p->add_jpeg_image(compressed_file, cinfo.image_width, cinfo.image_height,
                      MILIMETERS(margin_x_mm), MILIMETERS(margin_y_mm),
                      MILIMETERS(img_x_mm), MILIMETERS(img_y_mm),
                      cinfo.jpeg_color_space == JCS_GRAYSCALE ?
                      COLORSPACE_DEVICEGRAY : COLORSPACE_DEVICERGB);
    */
    p->add_image_bytes(image,
                       mask,
                       bit_depth,
                       channels,
                       x_size,
                       y_size,
                       MILIMETERS(margin_x_mm),
                       MILIMETERS(margin_y_mm),
                       MILIMETERS(img_x_mm),
                       MILIMETERS(img_y_mm),
                       (color_type == PNG_COLOR_TYPE_GRAY ||
                        color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ?
                       COLORSPACE_DEVICEGRAY : COLORSPACE_DEVICERGB,
                       true);

    return p;
}

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

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <document.h>

#include "create_page.h"

int main(int argc, char *argv[])
{
  std::vector<std::string> input_files;
  std::string output_file;
  std::vector<double> img_x_mm, img_y_mm;
  int quality = -1;
  bool shrink = true;

  // Default page size.
  double page_x_mm = 210.;
  double page_y_mm = 297.;

  for (auto i = 1; i < argc; ++i)
  {
    if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
    {
      std::cerr <<
        argv[0] << " embeds jpeg files on a PDF document.\n"
        "usage: " << argv[0] << " options\nwhere options are zero or more of:\n"
        "-i, --input file   input image name\n"
        "-x, --size-x mm    horizontal size of the last specified image\n"
        "-y, --size-y mm    vertical size of the last specified image\n"
        "-ns, --no-shrink   do not shrink the image to fit the page\n"
        "-o, --output file  output file name (if `-` or omitted, use stdout)\n"
        "-px, --page-x mm   width of the output pages (default: " << page_x_mm << ")\n"
        "-py, --page-y mm   height of the output pages (default: " << page_y_mm << ")\n"
        "-q, --quality int  quality of the output images (default: retain the input quality)\n"
        "-h, --help         show this message\n"
        "Sizes are specified in millimeters\n";

      return -2;
    }

    if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--input"))
    {
      input_files.push_back(std::string(argv[++i]));
      img_x_mm.push_back(-1.);
      img_y_mm.push_back(-1.);
    }

    if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output"))
    {
      output_file = std::string(argv[++i]);
    }

    if (!strcmp(argv[i], "-x") || !strcmp(argv[i], "--size-x"))
    {
      img_x_mm[img_x_mm.size() - 1] = atof(argv[++i]);
    }

    if (!strcmp(argv[i], "-y") || !strcmp(argv[i], "--size-y"))
    {
      img_y_mm[img_y_mm.size() - 1] = atof(argv[++i]);
    }

    if (!strcmp(argv[i], "-px") || !strcmp(argv[i], "--page-x"))
    {
      page_x_mm = atof(argv[++i]);
    }

    if (!strcmp(argv[i], "-py") || !strcmp(argv[i], "--page-y"))
    {
      page_y_mm = atof(argv[++i]);
    }

    if (!strcmp(argv[i], "-ns") || !strcmp(argv[i], "--no-shrink"))
    {
      shrink = false;
    }

    if (!strcmp(argv[i], "-q") || !strcmp(argv[i], "--quality"))
    {
      quality = atoi(argv[++i]);
    }
  }

  if (input_files.empty())
  {
    std::cerr << "Not enough arguments, use \"" << argv[0] << " --help\"." << std::endl;

    return -3;
  }

  paddlefish::DocumentPtr d(new paddlefish::Document());

  for (size_t i = 0; i < input_files.size(); ++i)
    d->push_back_page(empdfer::create_page(input_files[i], page_x_mm, page_y_mm,
                                           img_x_mm[i], img_y_mm[i], quality,
                                           shrink));

  if (output_file.empty() || output_file == "-")
  {
    d->to_stream(std::cout);
  }
  else
  {
    std::ofstream f(output_file, std::ios_base::out|std::ios_base::binary);
    d->to_stream(f);
    f.close();
  }

  return 0;
}

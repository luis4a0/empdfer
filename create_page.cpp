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

#include <exception>
#include <iostream>

#include "create_page.h"
#include "file_type.h"
#include "jpeg_file.h"
#ifdef EMPDFER_USE_PNG
#include "png_file.h"
#endif

paddlefish::PagePtr empdfer::create_page(const std::string& input_file,
                                         double page_x_mm, double page_y_mm,
                                         double img_x_mm, double img_y_mm,
                                         int quality, double rotation,
                                         bool shrink)
{
    switch (file_type(input_file))
    {
        case empdfer::FileType::JPEG:
            return empdfer::jpeg_page(input_file, page_x_mm, page_y_mm,
                                      img_x_mm, img_y_mm, quality, rotation,
                                      shrink);
            break;
        case empdfer::FileType::PNG:
#ifdef EMPDFER_USE_PNG
            return empdfer::png_page(input_file, page_x_mm, page_y_mm,
                                     img_x_mm, img_y_mm, quality, rotation,
                                     shrink);
#else
            throw std::runtime_error(input_file +
                ": PNG is not supported, compile with libpng");
#endif
            break;
        default:
            throw std::runtime_error(input_file + ": Unknown file type");
            break;
    }
}

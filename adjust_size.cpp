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

#include "adjust_size.h"

void empdfer::shrink(double *img_x_mm, double *img_y_mm, double page_x_mm,
                     double page_y_mm)
{
    if (*img_x_mm > page_x_mm)
        {
            *img_y_mm *= page_x_mm / *img_x_mm;
            *img_x_mm = page_x_mm;
        }

        if (*img_y_mm > page_y_mm)
        {
            *img_x_mm *= page_y_mm / *img_y_mm;
            *img_y_mm = page_y_mm;
        }

    return;
}
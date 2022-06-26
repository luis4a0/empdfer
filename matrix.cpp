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

#include "matrix.h"

#include <paddlefish/paddlefish.h>

#include <cmath>

namespace
{
double shrink_factor(double img_x_mm, double img_y_mm, double page_x_mm,
                     double page_y_mm)
{
  double factor = 1.;

  if (img_x_mm > page_x_mm)
    factor = page_x_mm / img_x_mm;

  if (img_y_mm * factor > page_y_mm)
    factor = page_y_mm / img_y_mm;

  return factor;
}
} // namespace

void empdfer::fill_matrix(double *matrix23, double img_x_mm, double img_y_mm,
                          double page_x_mm, double page_y_mm, double rotation,
                          bool shrink)
{
  // Compute the margins needed to center the image on page. For this, we
  // need to take the rotation into account.
  double r_normalized = fmod(rotation, 360.); // For simplicity.
  if (r_normalized < 0)
    r_normalized += 360.;
  double r_rad = std::acos(-1) * r_normalized / 180.; // acos(-1) is pi.
  double cos_r = std::cos(r_rad);
  double sin_r = std::sin(r_rad);

  // lab is the longitude over the "a" axis projected onto the "b" axis
  // after rotation.
  double lxx = std::abs(cos_r * img_x_mm);
  double lyx = std::abs(sin_r * img_y_mm);
  double lxy = std::abs(sin_r * img_x_mm);
  double lyy = std::abs(cos_r * img_y_mm);

  if (shrink)
  {
    double factor = shrink_factor(lxx + lyx, lxy + lyy, page_x_mm, page_y_mm);

    lxx *= factor;
    lyx *= factor;
    lxy *= factor;
    lyy *= factor;
    img_x_mm *= factor;
    img_y_mm *= factor;
  }

  // Compute margins as if the image is not rotated.
  double margin_x_mm = (page_x_mm - (lxx + lyx)) / 2.;
  double margin_y_mm = (page_y_mm - (lxy + lyy)) / 2.;

  if (r_normalized <= 90)
  {
    margin_x_mm += lyx;
  }
  else if (r_normalized <= 180)
  {
    margin_x_mm += (lxx + lyx);
    margin_y_mm += lyy;
  }
  else if (r_normalized <= 270)
  {
    margin_x_mm += lxx;
    margin_y_mm += (lxy + lyy);
  }
  else // r_normalized is > 270 and < 360.
  {
    margin_y_mm += lxy;
  }

  matrix23[0] = MILIMETERS(img_x_mm) * cos_r;
  matrix23[1] = MILIMETERS(img_x_mm) * sin_r;
  matrix23[2] = -MILIMETERS(img_y_mm) * sin_r;
  matrix23[3] = MILIMETERS(img_y_mm) * cos_r;
  matrix23[4] = MILIMETERS(margin_x_mm);
  matrix23[5] = MILIMETERS(margin_y_mm);

  return;
}

//
// Copyright (c) 2023 Humanitarian OpenStreetMap Team
//
// This file is part of Underpass.
//
//     Underpass is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Underpass is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Underpass.  If not, see <https://www.gnu.org/licenses/>.
//
#include <cmath>
#include <vector>
#include <utils/geo.hh>

/// \namespace geo
namespace geo {

void Geo::epsg4326toEpsg3857(double& x, double& y) {
  x = (x * 20037508.34) / 180;
  y = (std::log(std::tan(((90 + y) * M_PI) / 360))) /
      (M_PI / 180);
  y = (y * 20037508.34) / 180;
}

double Geo::calculateAngle(double x1, double y1, double x2, double y2, double x3, double y3) {
    Geo::epsg4326toEpsg3857(x1, y1);
    Geo::epsg4326toEpsg3857(x2, y2);
    Geo::epsg4326toEpsg3857(x3, y3);
    double ba0 = x1 - x2;
    double ba1 = y1 - y2;
    double bc0 = x3 - x2;
    double bc1 = y3 - y2;
    double dot_p = ba0 * bc0 + ba1 * bc1;
    double cosine_angle = dot_p / (
        std::pow((ba0 * ba0 + ba1 * ba1) , 0.5) *
        std::pow((bc0 * bc0 + bc1 * bc1) , 0.5)
    );
    double angle = acos(cosine_angle);
    return angle * 180 / M_PI;
}

} // EOF geo

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:



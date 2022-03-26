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

#include <algorithm>
#include <cctype>
#include <filesystem>

#include "file_type.h"

namespace {
inline bool ends_in(const std::string& s, const std::string& t)
{
    return std::string::npos != s.find(t, s.size() - t.size());
}
} // namespace

empdfer::FileType empdfer::file_type(const std::string& file_path)
{
    // Get rid of the folders in the path, keep only the file name.
    std::string name = std::filesystem::path(file_path).filename().string();

    // Make it lowercase (just to check extension).
    std::transform(file_path.begin(), file_path.end(), name.begin(), 
                   [](unsigned char c){ return std::tolower(c); });

    if (ends_in(name, ".jpg") || ends_in(name, ".jpeg"))
        return empdfer::JPEG;
    else if (ends_in(name, ".png"))
        return empdfer::PNG;
    else return empdfer::UNKNOWN;
}

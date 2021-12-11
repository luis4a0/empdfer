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

#ifndef EMPDFER_FILE_TYPE_H
#define EMPDFER_FILE_TYPE_H

#include <string>

namespace empdfer {

enum FileType
{
    JPEG,
    UNKNOWN
};

FileType file_type(const std::string&);

} // namespace empdfer

#endif // EMPDFER_FILE_TYPE_H
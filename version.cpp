// Copyright (c) 2023 Luis Peñaranda. All rights reserved.
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

#include "version.h"

std::string empdfer::version()
{
    return std::string("empdfer " + std::to_string(EMPDFER_VERSION_MAJOR) +
        "." + std::to_string(EMPDFER_VERSION_MINOR) + "." +
        std::to_string(EMPDFER_VERSION_PATCH) + " " +
#ifdef EMPDFER_WINDOWS
        "Windows"
#elif EMPDFER_MACOS
        "Mac"
#elif EMPDFER_LINUX
        "Linux"
#elif EMPDFER_FREEBSD
        "FreeBSD"
#else
        "unknown"
#endif
        );
}

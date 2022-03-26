# Copyright (c) 2021-2022 Luis Peñaranda. All rights reserved.
#
# This file is part of empdfer.
#
# Empdfer is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Empdfer is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with empdfer.  If not, see <http://www.gnu.org/licenses/>.

# When using valgrind, use -g in CXXPARAMS but do NOT use -ansi. It is also
# better to use -O0 in OPTIMIZATION in this case.

PDF_LIB=paddlefish
PDF_LIB_PATH=../../paddlefish/build
PDF_LIB_INCLUDE_PATH=../../paddlefish/build
EXT_LIBS_DEFS=-DPADDLEFISH_USE_ZLIB
CXXPARAMS=-ansi ${EXT_LIBS_DEFS} -Wall -pedantic -std=c++17
OPTIMIZATION=-O2
EXT_LIBS=-lm -lz -ljpeg -lpng

BINARY=empdfer

OBJECTS=file_type.o jpeg.o png_file.o create_page.o empdfer.o

%.o: %.cpp
	g++ ${CXXPARAMS} ${OPTIMIZATION} -I${PDF_LIB_INCLUDE_PATH} -c $< -o $@

all: ${PDF_LIB_PATH}/lib${PDF_LIB}.a ${BINARY}

empdfer: ${OBJECTS}
	g++ ${CXXPARAMS} ${OPTIMIZATION} -L${PDF_LIB_PATH} ${OBJECTS} -l${PDF_LIB} ${EXT_LIBS} -o $@

clean:
	rm -f *.o ${BINARY}

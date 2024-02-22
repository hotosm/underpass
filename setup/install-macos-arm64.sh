#!/bin/bash
#
# Copyright (c) 2024 Humanitarian OpenStreetMap Team
#
# This file is part of Underpass.
#
#     Underpass is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     Underpass is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with Underpass.  If not, see <https://www.gnu.org/licenses/>.

echo "Installing dependencies ..."
sudo port install boost 
brew install \
    libtool \
    gdal \
    pkg-config \
    openssl \
    protobuf \
    boost-python3 \
    libxml++3 \
    libpqxx \
    gumbo-parser
cd ..

echo "Setting up build ..."
./autogen.sh
mkdir build && cd build
../configure CXXFLAGS="-arch arm64 \
    -I/usr/local/include \
    -L/opt/homebrew/lib \
    -I/opt/homebrew/include \
    -I/opt/homebrew/Cellar/ \
    -L/usr/local/Cellar/ \
    -g -O2" CXX="g++"

echo "Building ..."
make -j$(nproc) && sudo make install
echo "Setting up libs ..."
ln -s /usr/local/lib/libboost_date_time.dylib .libs
ln -s /usr/local/lib/libboost_system.dylib .libs
ln -s /usr/local/lib/libboost_filesystem.dylib .libs
ln -s /usr/local/lib/libboost_log.dylib .libs
ln -s /usr/local/lib/libboost_program_options.dylib .libs
ln -s /usr/local/lib/libboost_iostreams.dylib .libs
ln -s /usr/local/lib/libboost_thread.dylib .libs 
ln -s /usr/local/lib/libboost_serialization.dylib .libs 
ln -s /usr/local/lib/libboost_regex.dylib .libs

echo "Done! now you may want to initialize the database with the bootstrap script"

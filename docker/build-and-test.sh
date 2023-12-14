#!/bin/bash

set -euo pipefail

yellow_echo() {
    local message="$1"
    local separator="--------------------------------------------------------"
    local sep_length=${#separator}
    local pad_length=$(( (sep_length - ${#message}) / 2 ))
    local pad=""

    for ((i=0; i<pad_length; i++)); do
        pad="$pad "
    done

    echo ""
    echo -e "\e[0;33m$separator\e[0m"
    echo -e "\e[0;33m$pad$message$pad\e[0m"
    echo -e "\e[0;33m$separator\e[0m"
    echo ""
}

# Generate makefiles
yellow_echo "Generating Makefiles"
./autogen.sh

echo
echo "Generating 'build' directory"
mkdir build
echo "Entering 'build' directory"
cd build

# Test build works
yellow_echo "Building Underpass"
../configure
make -j $(nproc)
make install

# Run tests
yellow_echo "Running Tests"
make check -j $(nproc)

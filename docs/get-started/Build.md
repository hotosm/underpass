# Build from source

## Linux

### Install dependencies

```sh
sudo apt-get update \
    && apt-get install -y software-properties-common \
    && apt-get update && apt-get install -y \
        libboost-dev \
        autotools-dev \
        swig \
        pkg-config \
        gcc \
        build-essential \
        ccache \
        libboost-all-dev \
        dejagnu \
        libjemalloc-dev \
        libxml++2.6-dev \
        doxygen \
        libgdal-dev \
        libosmium2-dev \
        libpqxx-dev \
        postgresql \
        libgumbo-dev \
        librange-v3-dev
```

### Build 

```sh
$ ./autogen.sh && \
  mkdir build && cd build && \ 
  ../configure && make -j$(nproc) && sudo make install
```

## MacOS (Intel)

### Install dependencies

```
brew install \
    autoconf \
    automake \
    libtool \
    swig \
    gdal \
    pkg-config \
    openjdk@11 \
    libosmium \
    gumbo-parser \
    boost \
    boost-python \
    open-mpi \
    range-v3 \
    wget \
    unzip \
    libxml++3 \
    libpqxx \
    ccache \
    openssl \
    protobuf
```

### Build 

```sh
sh ./autogen.sh
mkdir build && cd build
../configure CXXFLAGS="-I/usr/local/include \
    -I/usr/local/Homebrew/Cellar/libpqxx/7.7.5/include/ \
    -I/opt/homebrew/opt/openssl@3/include \
    -L/opt/homebrew/Cellar/boost-python3/1.82.0/lib \
    -L/opt/homebrew/Cellar/jemalloc/5.3.0/lib \
    -g -O0" CXX="ccache g++"
make -j$(nproc) && sudo make install
```

## MacOS (Sillicon)

Make sure you have installed all dependencies built for arm64 or as universal binaries.

You can do this with brew using:

`arch -arm64 brew install <package>`

Then the process is similar to Intel but adding `-arch arm64` to `CXXFLAGS`:

```sh
../configure CXXFLAGS="-arch arm64 -I/usr/local/include \
    -I/usr/local/Homebrew/Cellar/libpqxx/7.7.5/include/ \
    -I/opt/homebrew/opt/openssl@3/include \
    -L/opt/homebrew/Cellar/boost-python3/1.82.0/lib \
    -L/opt/homebrew/Cellar/jemalloc/5.3.0/lib \
    -g -O0" CXX="ccache g++"
```

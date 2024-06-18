# Build from source

## Linux

### Install Dependencies

If you want to build from source, you need to install the
dependencies. There are two types of dependencies, the build tools
needed to compile Underpass, and the other packages it needs.

#### Build Tools

The tools needed to build Underpass on a Ubuntu/Debian system are:

* git
* make
* pkg-config
* autoconf
* automake
* libtool
* doxygen
* g++
* build-essential
* software-properties-common

#### Developer Packages

This are the other development packages required to compile Underpass.

* python3-dev
* libssl-dev
* libgumbo-dev
* libopenmpi-dev
* libpqxx-dev
* libxml++2.6-dev
* libboost-all-dev
* libjemalloc-dev
* libgdal-dev
* libbz2-dev
* libosmium2-dev
* librange-v3-dev
* libboost-locale-dev
* libboost-date-time-dev
* libboost-thread-dev
* libboost-serialization-dev
* libboost-timer-dev
* libboost-filesystem-dev
* libboost-program-options-dev
* libboost-iostreams-dev

#### Building Libpqxx For Debian

Debian 12 (Bookworm) ships libpqxx-6.x, which has a bug that prevents
it from working with boost. This bug is fixed in libpqxx-7.x. Ubuntu
24.04 (noble) ships libpqxx-7.x, and Fedora >38 also ships the latest
version.

To build libpqxx, you can get the source here:
	https://salsa.debian.org/debian/libpqxx.git

Then install **cmake**, which is needed to configure libpqxx. By
default libpqxx doesn't build the shared libraries, so you have to
configure libpqxx like this:

	cmake .. -DBUILD_SHARED_LIBS=1
	make install

#### Install All Packages

On a Debian or Ubuntu system, you can install all the packages needed
like this. For Debian 12 or less, don't forget to build libpqxx
manually.

```sh
sudo apt-get update \
    && apt-get install -y software-properties-common \
    && apt-get update && apt-get install -y \
	git \
	pkg-config \
	autoconf \
	automake \
	libtool \
	doxygen \
	ccache \
	postgresql \
	g++ \
	python3-dev \
	libssl-dev \
	libgumbo-dev \
	libopenmpi-dev \
   	libxml++2.6-dev \
	libboost-all-dev \
	libjemalloc-dev \
	libgdal-dev \
	libbz2-dev \
	libosmium2-dev \
	librange-v3-dev \
	libboost-locale-dev \
	libboost-date-time-dev  \
	libboost-thread-dev \
	libboost-serialization-dev \
	libboost-timer-dev \
	libboost-filesystem-dev \
	libboost-program-options-dev \
	libboost-iostreams-dev
```

### Build Underpass

Underpass uses the GNU Autotools to configure itself. By default the
*configure* script is not include, it needs to be created by the
version of autotools on the build system. Building it is easy. Go to
the top level source directory and type **./autogen.sh**. This will
create the configure script.

The preferred way to compile Underpass is to create a build directory,
and run configure there. Once configured you can compile Underpass. If
you expect to be compiling Underpass frequently, you can optionally
install *ccache* so you don't have to compile everything everytime.

	export CXX='ccache g++'

```sh
$ mkdir build && cd build && \
  ../configure && make -j$(nproc) && sudo make install
```

#### Testing Underpass

If you want to run the Underpass tests, you need to install
**dejagnu**, and have a running **postgres** database. Postgresql is
of course required to run Underpass.

## MacOS

### Install dependencies

```
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
```

### Build (Intel)

```sh
sh ./autogen.sh
mkdir build && cd build
../configure CXXFLAGS=" \
    -I/usr/local/include \
    -L/opt/homebrew/lib \
    -I/opt/homebrew/include \
    -I/opt/homebrew/Cellar \
    -L/usr/local/Cellar" CXX="g++"
make -j$(nproc) && sudo make install
```

## MacOS (Sillicon)

The process is similar to Intel but adding `-arch arm64` to `CXXFLAGS`:

```sh
sh ./autogen.sh
mkdir build && cd build
../configure CXXFLAGS=" -arch arm64\
    -I/usr/local/include \
    -L/opt/homebrew/lib \
    -I/opt/homebrew/include \
    -I/opt/homebrew/Cellar \
    -L/usr/local/Cellar" CXX="g++"
make -j$(nproc) && sudo make install
```

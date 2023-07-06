# Underpass Docker installation

This is the easiest way to get Underpass up and running.

## 1.Select a region and update the installation script



## Install binary dependencies

```sh
$ sudo apt-get update && sudo apt-get install -y \
		autotools-dev \
		swig \
		python3-dev \
		libgdal-dev \
		pkg-config \
		openjdk-11-jdk \
		build-essential \
		libosmium2-dev \
		libgumbo-dev \
		libwebkit2gtk-4.0-dev \
		libopenmpi-dev \
		libboost-all-dev \
		librange-v3-dev \
		wget \
		unzip \
		libxml++2.6-dev \
		libpqxx-dev \
		libjemalloc-dev
```

## Compile Underpass

```sh
$ ./autogen.sh && \
  mkdir build && cd build && \ 
  ../configure && make -j4 && make install
```

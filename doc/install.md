# Underpass installation

Tested in operating system Ubuntu 20.04

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
		libxml++2.6-dev
```

## Compile libpqxx

### Create folder

```sh
$ mkdir /libpqxx && cd /libpqxx
```

### Download source dode

```sh
$ wget https://github.com/jtv/libpqxx/archive/7.5.2.zip && unzip 7.5.2.zip && cd libpqxx-7.5.2
```

### Compile as shared library

```sh
$ ./configure --enable-shared && make && sudo make install
```

## Compile Underpass

```sh
$ export PKG_CONFIG_PATH=$UNDERPASS_PATH/m4/
$ sudo ln -s /lib/x86_64-linux-gnu/pkgconfig/python-3.8.pc /lib/x86_64-linux-gnu/pkgconfig/python.pc
```

```sh
$ cd $UNDERPASS_PATH && ./autogen.sh && mkdir build && cd build && ../configure && make -j4
```

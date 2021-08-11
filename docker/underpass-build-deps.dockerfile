FROM ubuntu:focal

# This image is available as elpaso/underpass-build-deps:focal
LABEL maintainer="Humanitarian OpenStreetMap Team" Description="This image provides build deps for underpass" Vendor="HOT" Version="dev"

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y software-properties-common \
    && add-apt-repository -y ppa:ubuntugis/ubuntugis-unstable \
    && apt-get update && apt-get install -y \
        git \
        libboost-dev \
        autotools-dev \
        swig \
        python3 \
        python3-dev \
        libgdal-dev \
        pkg-config \
        gcc \
        build-essential \
        ccache \
        libosmium2-dev \
        libgumbo-dev \
        libwebkit2gtk-4.0-dev \
        libopenmpi-dev \
        libboost-all-dev \
        librange-v3-dev \
        libyaml-cpp-dev \
        dejagnu \
        wget \
        unzip \
        libxml++2.6-dev && rm -rf /var/lib/apt/lists/* \
        vim \
        wait-for-it \
    && mkdir /libpqxx && cd /libpqxx \
    && wget https://github.com/jtv/libpqxx/archive/7.6.0.zip \
    && unzip 7.6.0.zip \
    && cd libpqxx-7.6.0 \
    && ./configure --enable-shared \
    && make \
    && make install \
    && cd .. \
    && rm -rf libpqxx-7.6.0 \
    && rm 7.6.0.zip \
    && rm -rf /var/lib/apt/lists/*

COPY bzip2.pc /usr/lib/x86_64-linux-gnu/pkgconfig/bzip2.pc


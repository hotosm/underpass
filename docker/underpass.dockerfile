FROM ubuntu:kinetic

# This image is available as quay.io/hotosm/underpass:kinetic
LABEL maintainer="Humanitarian OpenStreetMap Team" Description="This image provides build deps for underpass" Vendor="HOT" Version="dev"

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y software-properties-common \
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
        libjemalloc-dev \
        wget \
        unzip \
        libxml++2.6-dev && rm -rf /var/lib/apt/lists/* \
        liblua5.3-dev \
        vim \
        wait-for-it \
        cmake \
        doxygen

RUN apt-get update && apt-get -y install libpqxx-dev

RUN apt-get update && apt-get -y install postgresql

COPY docker/bzip2.pc /usr/lib/x86_64-linux-gnu/pkgconfig/bzip2.pc
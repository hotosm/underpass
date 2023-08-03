FROM ubuntu:kinetic

# This image is available as quay.io/hotosm/underpass:kinetic
LABEL maintainer="Humanitarian OpenStreetMap Team" Description="This image provides a build for Underpass" Vendor="HOT" Version="dev"
ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /code

COPY ./src /code/src
COPY ./config /code/config
COPY ./setup /code/setup

RUN apt-get update \
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

COPY ./docker/bzip2.pc /usr/lib/x86_64-linux-gnu/pkgconfig/bzip2.pc
COPY ./autogen.sh /code/autogen.sh
COPY ./configure.ac /code/configure.ac
COPY ./Makefile.am /code/Makefile.am
COPY ./m4 /code/m4
COPY ./dist /code/dist
COPY ./docs /code/docs
COPY ./ABOUT-NLS /code/ABOUT-NLS
COPY ./config.rpath /code/config.rpath

WORKDIR /code
RUN ./autogen.sh 

WORKDIR /code/build 
RUN ../configure && \
    make -j $(nproc) && \
    make install


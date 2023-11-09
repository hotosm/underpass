FROM ubuntu:latest

FROM docker.io/debian:bookworm-slim as base
ARG APP_VERSION
ARG COMMIT_REF
LABEL org.hotosm.underpass.app-name="underpass" \
      org.hotosm.underpass.app-version="${APP_VERSION}" \
      org.hotosm.underpass.git-commit-ref="${COMMIT_REF:-none}" \
      org.hotosm.underpass.maintainer="sysadmin@hotosm.org"
RUN set -ex \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install \
    -y --no-install-recommends "locales" "ca-certificates" \
    && DEBIAN_FRONTEND=noninteractive apt-get upgrade -y \
    && rm -rf /var/lib/apt/lists/* \
    && update-ca-certificates
# Set locale
RUN sed -i '/en_US.UTF-8/s/^# //g' /etc/locale.gen && locale-gen
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8



FROM base as build

RUN set -ex \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install \
    -y --no-install-recommends \
        "software-properties-common" \
        "libboost-all-dev" \
        "autotools-dev" \
        "swig" \
        "pkg-config" \
        "gcc" \
        "build-essential" \
        "ccache" \
        "dejagnu" \
        "libjemalloc-dev" \
        "libxml++2.6-dev" \
        "doxygen" \
        "libgdal-dev" \
        "libosmium2-dev" \
        "libpqxx-dev" \
        "postgresql" \
        "libgumbo-dev" \
        "librange-v3-dev" \
        "libtool" \
    && rm -rf /var/lib/apt/lists/*

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




FROM base as runtime
ENV PATH=$PATH:/code/build \
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/code/build/.libs
RUN set -ex \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install \
    -y --no-install-recommends \
        "libboost-all-dev" \
        "libgdal32" \
        "libxml++2.6-2v5" \
        "libjemalloc2" \
        "libpqxx-6.4" \
        "libgumbo1" \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /code/build /code/build
WORKDIR /code/build
# Add non-root user
RUN useradd -r -u 1001 -m -c "hotosm account" -d /home/appuser -s /bin/false appuser
# Change to non-root user
USER appuser
# ENTRYPOINT ["underpass"]

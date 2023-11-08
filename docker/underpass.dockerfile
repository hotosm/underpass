# Copyright (c) 2022, 2023 Humanitarian OpenStreetMap Team
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
#     along with Underpass.  If not, see <https:#www.gnu.org/licenses/>.
#

FROM docker.io/debian:bookworm-slim as base
ARG APP_VERSION
ARG COMMIT_REF
LABEL org.hotosm.fmtm.app-name="underpass" \
      org.hotosm.fmtm.app-version="${APP_VERSION}" \
      org.hotosm.fmtm.git-commit-ref="${COMMIT_REF:-none}" \
      org.hotosm.fmtm.maintainer="sysadmin@hotosm.org"
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
ENV PATH=$PATH:/app \
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/app/.libs
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
COPY --from=build /code/build /app
WORKDIR /app
# Add non-root user
RUN useradd -r -u 1001 -m -c "hotosm account" -d /home/appuser -s /bin/false appuser
# Change to non-root user
USER appuser
# ENTRYPOINT ["underpass"]

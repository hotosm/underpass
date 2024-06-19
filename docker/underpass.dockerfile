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



FROM base as deps
# Install dev deps
WORKDIR /code
RUN set -ex \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install \
    -y --no-install-recommends \
        "git" \
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
        "postgresql-postgis" \
        "libgumbo-dev" \
        "librange-v3-dev" \
        "libtool" \
        "osm2pgsql" \
        "rsync" \
    && rm -rf /var/lib/apt/lists/*



FROM deps as build
# Build underpass
COPY . .
RUN ./autogen.sh
WORKDIR /code/build
RUN ../configure && \
    make -j $(nproc) && \
    make install



FROM build as ci
ENV LD_LIBRARY_PATH=/code/build/.libs/
WORKDIR /code/build
ENTRYPOINT [ "/bin/bash", "-c" ]
CMD ["make", "check", "-j", "$(nproc)"]



FROM deps as debug
# Local debug with all dev deps
ENV LD_LIBRARY_PATH=/code/build/.libs/
COPY --from=build /usr/local/lib/ /usr/local/lib/
COPY --from=build /usr/local/bin /usr/local/bin
COPY --from=build /code/build/.libs/ \
    /code/build/.libs/
COPY --from=build /etc/underpass \
    /etc/underpass
WORKDIR /code/build
# Add non-root user
RUN useradd -r -u 1001 -m -c "hotosm account" -d /home/appuser -s /bin/false appuser
# Change to non-root user
USER appuser



FROM base as prod
# Production image with minimal deps
ENV LD_LIBRARY_PATH=/usr/local/lib
RUN set -ex \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install \
    -y --no-install-recommends \
        "libboost-all-dev" \
        "libgdal32" \
        "libosmium2-dev" \
        "libxml++2.6-2v5" \
        "libjemalloc2" \
        "libpqxx-6.4" \
        "libgumbo1" \
        "postgresql-client" \
        "osm2pgsql" \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /usr/local/bin /usr/local/bin
COPY --from=build /usr/local/lib/ \
    /usr/local/lib/
COPY --from=build /etc/underpass \
    /etc/underpass
WORKDIR /code/build
# Add non-root user
RUN useradd -r -u 1001 -m -c "hotosm account" -d /home/appuser -s /bin/false appuser
# Change to non-root user
USER appuser
# ENTRYPOINT ["underpass"]

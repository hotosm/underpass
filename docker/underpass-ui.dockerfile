FROM node:alpine 

LABEL maintainer="Humanitarian OpenStreetMap Team" Description="This image provides the Underpass UI playground" Vendor="HOT" Version="dev"

FROM docker.io/node:${NODE_TAG}-bookworm-slim as base
ARG APP_VERSION
ARG COMMIT_REF
LABEL org.hotosm.underpass.app-name="underpass-ui" \
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
RUN set -ex \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install \
    -y --no-install-recommends \
        "git" \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /code
RUN git clone https://github.com/hotosm/underpass-ui.git .
RUN yarn install
RUN yarn build
# Cosmos
WORKDIR /code/playground
RUN yarn install



FROM deps as build
RUN yarn run cosmos:export



FROM deps as debug
CMD yarn run cosmos



FROM docker.io/devforth/spa-to-http:1.0.3 as prod
WORKDIR /app
# Add non-root user, permissions
RUN adduser -D -u 1001 -h /home/appuser appuser
USER appuser
COPY --from=build --chown=appuser:appuser /repo/playground/cosmos-export/ .

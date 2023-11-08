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
ARG NODE_TAG=18


FROM docker.io/node:${NODE_TAG}-bookworm-slim as base
ARG APP_VERSION
ARG COMMIT_REF
LABEL org.hotosm.fmtm.app-name="underpass-ui" \
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
        "git" \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /repo
RUN git clone https://github.com/hotosm/underpass-ui.git .
# UI
RUN yarn install
RUN yarn build
# Cosmos
WORKDIR /repo/playground
RUN yarn install
RUN yarn run cosmos:export



FROM docker.io/devforth/spa-to-http:1.0.3 as prod
WORKDIR /app
# Add non-root user, permissions
RUN adduser -D -u 1001 -h /home/appuser appuser
USER appuser
COPY --from=build --chown=appuser:appuser /repo/playground/cosmos-export/ .

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
ARG PYTHON_TAG=3.10

FROM docker.io/python:${PYTHON_TAG}-slim-bookworm as base
ARG APP_VERSION
ARG COMMIT_REF
LABEL org.hotosm.fmtm.app-name="underpass-api" \
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
        "build-essential" \
        "libpq-dev" \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/python
COPY python/dbapi/requirements.txt /opt/python/requirements.txt
COPY python/restapi/requirements.txt /opt/python/requirements2.txt
RUN pip install --user --no-warn-script-location --no-cache-dir \
    -r /opt/python/requirements.txt -r /opt/python/requirements2.txt



FROM base as runtime
ARG PYTHON_TAG
ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    PYTHONFAULTHANDLER=1 \
    PATH="/home/appuser/.local/bin:$PATH" \
    PYTHONPATH="/opt/restapi" \
    PYTHON_LIB="/home/appuser/.local/lib/python$PYTHON_TAG/site-packages" \
    SSL_CERT_FILE=/etc/ssl/certs/ca-certificates.crt \
    REQUESTS_CA_BUNDLE=/etc/ssl/certs/ca-certificates.crt \
    CURL_CA_BUNDLE=/etc/ssl/certs/ca-certificates.crt
RUN set -ex \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install \
    -y --no-install-recommends \
        "postgresql-client" \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build \
    /root/.local \
    /home/appuser/.local

COPY /python/dbapi /opt/dbapi
COPY /python/restapi /opt/restapi
WORKDIR /opt/restapi

# Add non-root user, permissions
RUN useradd -r -u 1001 -m -c "hotosm account" -d /home/appuser -s /bin/false appuser \
    && chown -R appuser:appuser /opt /home/appuser
# Change to non-root user
USER appuser
# Add Healthcheck
HEALTHCHECK --start-period=10s --interval=5s --retries=12 --timeout=5s \
    CMD curl --fail http://localhost:8000 || exit 1

ENTRYPOINT ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]

FROM python:3.9

FROM docker.io/python:${PYTHON_TAG}-slim-bookworm as base
ARG APP_VERSION
ARG COMMIT_REF
LABEL org.hotosm.underpass.app-name="underpass-api" \
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

WORKDIR /code

RUN apt-get update && apt-get -y install \ 
    postgresql \
    libpq-dev

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
COPY /python/dbapi /code/dbapi
COPY /python/restapi /code/restapi
WORKDIR /code/restapi
# Add non-root user, permissions
RUN useradd -r -u 1001 -m -c "hotosm account" -d /home/appuser -s /bin/false appuser \
    && chown -R appuser:appuser /code /home/appuser
# Change to non-root user
USER appuser
# Add Healthcheck
HEALTHCHECK --start-period=10s --interval=5s --retries=12 --timeout=5s \
    CMD curl --fail http://localhost:8000 || exit 1

FROM runtime as debug
CMD ["uvicorn", "main:app", \
    "--host", "0.0.0.0", "--port", "8000", \
    "--reload", "--log-level", "critical", "--no-access-log"]

FROM runtime as prod
# Pre-compile packages to .pyc (init speed gains)
RUN python -c "import compileall; compileall.compile_path(maxlevels=10, quiet=1)"
# Note: 4 uvicorn workers as running with docker, change to 1 worker for Kubernetes
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000", \
    "--workers", "4", "--log-level", "critical", "--no-access-log"]

#!/bin/bash

# Runs the CI workflow locally using docker-compose, the issued commands
# MUST be kept in sync with those in .github/workflows/run_tests.yml


TEMP_DIR=`mktemp -d -t underpass_ci_local_XXXX`
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

echo "Running Underpass CI tests in temporary directory: ${TEMP_DIR}"


cp -r ${SCRIPT_DIR}/.. ${TEMP_DIR}
pushd ${PWD}

cd ${TEMP_DIR}

# Just in case
make distclean -j `nproc` || true

DOCKER_DIR="${TEMP_DIR}/docker"
DOCKER_COMPOSE_FILE="${DOCKER_DIR}/docker-compose.yml"
DOCKER_BASE_COMMAND="docker-compose -f ${DOCKER_COMPOSE_FILE}"

# Run the composition
${DOCKER_BASE_COMMAND} up -d

# Build Underpass Library and Binaries
${DOCKER_BASE_COMMAND} exec -T underpass-build-deps sh -c "cd /code && ./autogen.sh && (rm -rf build || true) && mkdir build && cd build && ../configure && make -j `nproc`"

# Build and Run Underpass Tests - broken: alway succeeds
${DOCKER_BASE_COMMAND} exec -T underpass-build-deps sh -c "cd /code/build/testsuite/libunderpass.all && make check -j `nproc`"

# Comment the cleanup lines below or exit here if you want to run additional
# tests from a console in the temp container, for instance with:
${DOCKER_BASE_COMMAND} exec underpass-build-deps bash

${DOCKER_BASE_COMMAND} down
echo "Remove temporary folder ${TEMP_DIR}"
sudo rm -rf ${TEMP_DIR}

popd
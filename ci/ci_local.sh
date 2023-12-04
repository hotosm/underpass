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
make clean -j `nproc` || true

# Run CI
APP_VERSION=ci docker compose run underpass --exit-code-from=underpass

# Shut down containers
APP_VERSION=ci docker compose down

echo "Remove temporary folder ${TEMP_DIR}"
sudo rm -rf ${TEMP_DIR}

popd

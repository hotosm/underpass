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

# Stop containers if running
docker rm --force underpass underpass_postgis underpass_api underpass_ui || true

# Run CI
export TAG_OVERRIDE=ci
echo
# Use -t flag to pass signals via tty
docker compose run -t --rm --build underpass
echo "Returned signal from tests: $?"
echo

echo "Remove temporary folder ${TEMP_DIR}"
sudo rm -rf ${TEMP_DIR}

popd

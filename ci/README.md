## Continuous integration scripts

Support scripts that related to the continuous integration (CI) process:

- `ci_local.sh` run the CI workflow locally using docker and docker compose, note that it will re-run configure on you main source tree.

## Github action workflows

There are three github actions. 

1. Doxygen Action (`.github/workflows/main.yml`)
  1. Runs on push to the master branch 
  2. Generates Doxygen docs and pushes to gh-pages branch, which hosts the docs
2. Deps Dockerfile Build and Push (`.github/workflows/dockerfile.yml`)
  1. Runs on feature branch `update/docker-ci` only
  2. Builds and runs the dependencies docker image. It will only push a new image if there are changes to the Dockerfile. 
3. 🧪 Build and Test (`.github/workflows/run_tests.yml`)
  1. Runs on push to any branch, and pull requests. This means sometimes it runs twice
  2. Runs tests. These should be kept aligned with those in `ci/ci_local.sh`
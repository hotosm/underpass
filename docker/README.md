# Docker images and compose setup for Underpass building and CI testing

+ `underpass-build-deps.dockerfile` is the recipe for Underpass build dependencies
+ `docker-compose.yml` runs two containers:
  + postgis: for Underpass I/O test DBs (DBs are intially empty)
  + underpass-build-deps for Underpass build and test execution

A pre-built image of `underpass-build-deps.dockerfile` is available on Docker Hub as `elpaso/underpass-build-deps:focal`.

## Build image

`docker build -t elpaso/underpass-build-deps:focal -f underpass-build-deps.dockerfile .`

## Docker compose

The docker composition is used in a GH workflow for continuous integration testing.

Inside the containers, the source code tee is mounted as `/code`/.

### Running the composition

```bash
$ cd docker
$ docker-compose up
```

### Running commands in the container

Commands can be executed from the host in the containers, for example you can open a shell in the container with:

```bash
$ docker-compose -f docker-compose.yml exec bash
```

# Docker images and compose setup for Underpass building and CI testing

+ `underpass.dockerfile` is the recipe for Underpass build dependencies
+ `docker-compose.yml` runs two containers:
  + postgis: for Underpass I/O test DBs (DBs are intially empty)
  + underpass for Underpass build and test execution

A pre-built image of `underpass.dockerfile` is available on Docker Hub as `quay.io/hotosm/underpass:kinetic`.

## Build image

`docker build -t quay.io/hotosm/underpass:kinetic -f underpass.dockerfile .`

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

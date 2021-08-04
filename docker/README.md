## Docker images and compose setup for Underpass building and CI testing

+ `underpass-build-deps.dockerfile` is the recipe for Underpass build dependencies
+ `docker-compose.yml` runs two containers:
  + postgis: for Underpass I/O test DBs
  + underpass-build-deps for Underpass build and test execution

The docker composition is used in a GH workflow for continuous integration testing.



## Test Data

This directory contains data files for CI tests.


`simple_test.osm` contains a simple OSM XML test file (from GDAL test suite), this file
can be transformed in the data file used for `osm2pgsql` tests by running:

```
osm2pgsql \
    --slim \
    -d pgsql_pbf_import_test \
    --output=pgsql \
    -l \
    --hstore-all \
    --middle-schema=osm2pgsql_pgsql \
    --output-pgsql-schema=osm2pgsql_pgsql \
    --extra-attributes \
    --create \
    -r xml \
    simple_test.osm

```

Then, a dump must be created with:

`pg_dump --inserts -O -a pgsql_pbf_import_test | gzip > pgsql_test_data.sql.gz`

`simple_test.osc` contains manually generated changes for `osm2pgsql` tests.

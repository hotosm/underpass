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
    --middle-schema=public \
    --output-pgsql-schema=public \
    --extra-attributes \
    --create \
    -r xml \
    test_initial.osm

```

Note that a DB named `pgsql_pbf_import_test` with `postgis` and `hstore` extensions and
`public` schema must exist, you can change `osm2pgsql` to suit your needs in case
you are using a different DB.

Then, a dump must be created with:

`pg_dump --inserts -O -a pgsql_pbf_import_test | gzip > osm2pgsql_test_data.sql.gz`

The schema can be exported with:

`pg_dump -O -s pgsql_pbf_import_test > osm2pgsql_test_schema.sql`



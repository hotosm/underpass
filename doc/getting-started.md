# Getting started with Underpass

The easiest way to start using Underpass is with [Docker](https://docs.docker.com/get-docker/) and [docker-compose](https://docs.docker.com/compose/install/).


## Build

_If you don't want to use Docker, see https://github.com/hotosm/underpass/blob/master/doc/install.md and
skip this section._

After cloning the project from https://github.com/hotosm/underpass , run:

    cd docker && docker-compose up -d

Now you can login into the container:

    docker exec -w /code -ti underpass_build  /bin/bash 

And once inside the container's `/code` directory, build Underpass:

    ./autogen.sh && mkdir build && cd build && ../configure && make -j2

The last parameter `-j2` is for speeding up the building process (is the number of jobs run in parallel). This value depends on your hardware, you can try using the value shown at `echo $(nproc)` and add 1 to it,  but this is not a guarantee. If you see errors during the build process
you can try with a smaller number.

## Setting up a replicator + validation process

The replicator will download data from OSM planet servers and save the validation results in a database.

### 1. Install PostgreSQL, create a database and tables for data output

    apt update && apt-get -y install postgresql postgresql-contrib
    psql -U underpass_test -c 'create database galaxy_test'
    cd /code/data && psql -U underpass_test galaxy_test < galaxy.sql

_Galaxy is a work in progress project focused on bringing together all of the OSM data outputs under one umbrella (https://galaxy.hotosm.org/)_

### 1. Create directories and copy configuration files for validation:

    cd /code/build && mkdir /usr/local/lib/underpass/ && mkdir replication 
    cp ../validate/*.yaml /usr/local/lib/underpass/

### 3. Run replicator
    
    ./replicator -v -l -m -f minute -t 2022-01-01T00:00:00  --server underpass_test@postgis/galaxy_test

Once replicator start running, it will save cache files in the `replication` directory and save the validation results into the database. 

For running the replicator using a changeset URL instead of a timestamp, use the `-u` option:
    
    ./replicator -v -l -m -f minute -u 004/911/945 --server underpass_test@postgis/galaxy_test

For more options:

    ./replicator --help

## Validation

Connect to the database you've created before:

`psql -U underpass_test galaxy_test`

And query the validation table to see what things Underpass is finding out:

`select * from validation limit 50;`

If you want to connect to the DB from your host computer, use this configuration:

    host: localhost
    database: galaxy_test
    user: underpass_test
    password: underpass_test

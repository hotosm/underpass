## Docker installation

This is the easiest way to get Underpass up and running:

```sh
docker-compose up -d
```

## Setup and run demo

### Bootstrap the database for a country

Requirements:

- fiona (pip install fiona)
- shapely (pip install shapely)
- osm2pgsql (https://osm2pgsql.org/doc/install.html)
- psql (https://www.postgresql.org/download/)

Run this utility script for bootstrap the database with data for a country:

```sh
cd setup && ./bootstrap.sh -r asia -c nepal -p 5439
```
Regions (-r) are:

* africa
* asia
* australia-oceania
* central-america
* europe
* north-america
* south-america

Countries (-c) is the name of the country inside the region.

Data is downloaded from [GeoFabrik](https://download.geofabrik.de/), if you are not sure of what name you need to use, please check there.

### Configure the UI

You can find the UI's playground here: `http://localhost:8080`

Edit this file with the center coordinates for the map:

`js/src/fixtures/center.js`

And copy it to the container:

```sh
docker cp js/src/fixtures/center.js underpass_ui:/code/src/fixtures/center.js
```

Refresh the page and you'll see your map centered in the new coordinates,
displaying raw data and validation results on top of the OSM map.

### Keep the database up-to-date minutely

Run this command for start processing data from 2 days ago:

```sh
docker exec -d underpass underpass -t $(date +%Y-%m-%dT%H:%M:%S -d "2 days ago")
```

From MacOS, the date command is different:

```sh
docker exec -d underpass underpass -t $(date -v -2d +%Y-%m-%dT%H:%M:%S)
```

For running underpass as a daemon, use the `-d` option:

```sh
docker exec -d underpass underpass -t $(date -v -2d +%Y-%m-%dT%H:%M:%S)
```

### Stop and start underpass

If you want to stop the process, you can kill the container:

```sh
docker kill underpass
```

Then, you'll have to run `docker-compose up -d` before starting the process again.


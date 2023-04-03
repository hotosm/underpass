# Underpass

Underpass is a **data analysis engine** that process OpenStreetMap data and provides customizable **statistics and validation** reports in **near real time**.

Is light, fast, easy to install and it can run on small systems.

## Quick start: install with Docker

The easiest way to start using Underpass is with [Docker](https://docs.docker.com/get-docker/) and [docker-compose](https://docs.docker.com/compose/install/).

_If you don't want to use Docker, you can install dependencies and build Underpass yourself. See  https://github.com/hotosm/underpass/blob/master/doc/install.md_

```
    git clone https://github.com/hotosm/underpass.git
    cd underpass/docker && sh install-with-docker.sh
```

## Process OsmChanges

Just run the Underpass replicator program with a date range:

```
docker exec -t underpass ./replicator -t 2022-01-01 -t 2022-01-02
```

A start date:

```
docker exec -t underpass ./replicator -t 2022-01-01 -t 2022-01-02
```

Or pass the value `now`:

```
docker exec -t underpass ./replicator -t now
```

## Get results

### Stats

Get stats for added, modified and hashtags and export them to a CSV file in `underpass/build/stats.csv`.

```
docker exec -t underpass psql --csv -t -d postgresql://underpass@postgis/underpass -c '\copy (SELECT id,added,modified,hashtags FROM changesets) to /code/build/stats.csv'
```

### Validation

Get ChangeSet ids and validation results and export them to a the CSV file in `underpass/build/validation.csv`.

```
docker exec -t underpass psql --csv -t -d postgresql://underpass@postgis/underpass -c '\copy (SELECT change_id, status FROM validation WHERE 'correct' != ANY (status);) to /code/build/validation.csv'
```

### Database

You can also use the connection string `postgresql://underpass@postgis/underpass` to connect to the DB with any PostregSQL client.

## Customization

### Stats

Collected stats can customized to your needs, changing the stats configuration YAML in [validate/statistics](https://github.com/hotosm/underpass/blob/master/validate/statistics.yaml).

You can find more information about stats [here](docs/statistics.md)

### Validation

Similar to stats, you can edit the YAML files inside the `validate` directory.

You can find more information about validation [here](docs/validation.md)

### Filter by area

To collect data for an specific area, create a GeoJSON file with the area you want. This file must contain a single MultiPolygon, like [this one](https://github.com/hotosm/underpass/blob/master/config/replicator/priority.geojson).

Then, just copy the file to `underpass/build` and pass the `--boundary` option:

```
replicator -u 2022-01-01 2022-01-02 --boundary=map.geojson
```



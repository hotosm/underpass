![CI Build and Testing](https://github.com/hotosm/underpass/actions/workflows/run_tests.yml/badge.svg)
![Doxygen](https://github.com/hotosm/underpass/actions/workflows/main.yml/badge.svg)

# Underpass

Underpass is a customizable **data engine** that processes **OpenStreetMap** data.

It provides customizable **statistics** and **validation** reports and it can also be used to
update a local copy of the OSM database. It is designed to be **high performance** on modest hardware.

## Quick start

```sh
git clone https://github.com/hotosm/underpass.git
sh docker/install.sh
```

After installation is done, a process will start downloading and processing
OSM data from a week ago, storing the results in the database and keep running
for updating data every minute.

You can start/stop the replication process:

```sh
sh docker/services-start.sh
sh docker/services-stop.sh
```

If you want to avoid using Docker and build Underpass on your system, check
the [install](https://github.com/hotosm/underpass/blob/master/doc/install.md) 
documentation.

## Using the data

### Reports on your browser

Open http://127.0.0.1:5000 on your browser and you'll see a set of UI components
for OSM data analysis.

### REST API

You can request the REST API directly:

```sh
curl --location 'http://127.0.0.1:8000/report/dataQualityTag/csv' \
--header 'Content-Type: application/json' \
--data '{
    "fromDate": "2023-01-01T00:00:00",
    "hashtags": []
}'
```

### DB API

See an example of how generate reports using the DB API:

`docker exec -w /code/util/python/dbapi/example -t underpass python csv-report.py`

### Python API

Use the Python example for download and analyze a Changeset:

```sh
docker exec -w /code/util/python/examples -t underpass \
python validation.py -u https://www.openstreetmap.org/api/0.6/changeset/133637588/download -c place
```

## Get involved!

We invite software designers and developers to contribute to the project, there are several issues
where we need help, some of them are:

* Designs for data visualizations
* React UI components
* Data quality checks for the C++ core engine
* PostgreSQL queries for the Python `dbapi` module
* Endpoints for the Python `restapi` module
* Packages for Python, React and system binaries
* Data models for semantic validation
* Tests for everything

### Roadmap

Below there's reference to the Underpass Product Roadmap (subject to change).

<img width="810" alt="Screenshot 2023-04-07 at 10 34 22" src="https://user-images.githubusercontent.com/1226194/230617809-8d5a2757-3ba8-4097-b03e-650364f75dd5.png">

### Core documentation

Check the [docs](https://hotosm.github.io/underpass/annotated.html) for
internal documentation of all the C++ classes.


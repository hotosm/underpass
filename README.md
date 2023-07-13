![CI Build and Testing](https://github.com/hotosm/underpass/actions/workflows/run_tests.yml/badge.svg)
![Doxygen](https://github.com/hotosm/underpass/actions/workflows/main.yml/badge.svg)

# Underpass

Underpass is a customizable **data engine** that processes **OpenStreetMap** data.

It **updates a local copy of the OSM database** and provides customizable **statistics** and **validation** reports. It is designed to be **high performance** on modest hardware.

## Demo

We've deployed a rudimentary demo that keeps a database up-to-date for (some country),
rendering buildings and highlighting the ones identified as "un-squared":

~[http://underpass.live:5000](http://underpass.live:5000/?fixtureId={"path"%3A"src%2Ffixtures%2FDQMap.fixture.jsx"})~

<img width="870" alt="Screenshot 2023-07-12 at 22 28 23" src="https://github.com/hotosm/underpass/assets/1226194/19f90e7a-d51c-4aa0-8f76-5155966587c5">

<img width="872" alt="Screenshot 2023-07-12 at 22 31 02" src="https://github.com/hotosm/underpass/assets/1226194/612894d5-4988-45fb-9aa0-4618b1a6e67e">

## Getting started

If you want to get started really quick and easy, use the [Docker installation](https://github.com/hotosm/underpass/blob/master/docs/install-docker.md) and skip the next steps.

### 1. Install

Install the software on your platform of preference.

* [Linux](https://github.com/hotosm/underpass/blob/master/docs/install.md)
* MacOS (docs in progress ...)

### 2. Setup

Select the region you want to work with and bootstrap the database with data.

* [Using the bootstrap.sh script](https://github.com/hotosm/underpass/blob/master/docs/bootstrapsh.md)

### 3. Run

For keeping the database up-to-date, you must run underpass:

`./underpass -t $(date +%Y-%m-%dT%H:%M:%S -d "2 days ago")'`

On MacOS, the date command works different:

`./underpass -t $(date -v -2d +%Y-%m-%dT%H:%M:%S)`

A process will start downloading and processing OSM data until lastest data
is reached, and then it will continue updating data every minute.

## Using the data

Aside of querying the database, there are two utilities that will make your live
easier.

* [Install & run the Underpass Python REST API](https://github.com/hotosm/underpass/blob/master/docs/python-rest-api.md)
* [Use the Underpass UI components](https://github.com/hotosm/underpass/blob/master/docs/ui-components.md)

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

### Documentation

* Check the [docs](https://github.com/hotosm/underpass/tree/master/docs) folder.
* For internal documentation of all the C++ classes: [docs](https://hotosm.github.io/underpass/annotated.html) 

### License

Underpass is free software! you may use any Underpass project under the terms of
the GNU General Public License (GPL) Version 3.

![CI Build and Testing](https://github.com/hotosm/underpass/actions/workflows/run_tests.yml/badge.svg)
![Doxygen](https://github.com/hotosm/underpass/actions/workflows/docs.yml/badge.svg)

# Underpass

Underpass is a customizable **data engine** that processes **mapping** data.

It **updates a local copy of the OSM database** in near real-time, and provides customizable **statistics** and **validation** reports. It is designed to be **high performance** on modest hardware.

<img width="1483" alt="Screenshot 2023-09-01 at 07 44 19" src="https://github.com/hotosm/underpass/assets/1226194/09a942a0-9f03-49d1-9cfe-7359b764441b">

## Demo

We've deployed a rudimentary demo that keeps a database up-to-date for (some country),
rendering buildings and highlighting the ones identified as "un-squared":

[See The demo](https://underpass.hotosm.org)

## Getting started

Check [The docs](https://hotosm.github.io/underpass/)!

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

### License

Underpass is free software! you may use any Underpass project under the terms of
the GNU General Public License (GPL) Version 3.

# Utility Programs

## data/db/setupdb.sh

This is a simple shell script that creates all the databases Underpass
uses. It can also be used to initialize the two internal databases
Underpass uses, one for the *.state.txt files that contain the
timestamp of the data files, and the other is for the boundaries of
countries used to determine which country a change was made in for
statistics collection. These are used to bootstrap a new Underpass
installation.

## setup/bootstrap.sh

* [Using the bootstrap.sh script](/underpass/Dev/bootstrapsh)

## utils/features2yaml

Get tags from the Map Features OSM wiki and output YAML

Use this script for output a YAML data model that will be used in 
the configuration files inside /validation directory.

### Usage

```sh
python features2yaml.py --category buildings > buildings.yaml
python features2yaml.py --key building:material > building:material.yaml
python features2yaml.py --url https://wiki.openstreetmap.org/wiki/Key:landuse
python features2yaml.py --f ../../place.html
```

You may want to add more configuration parameters to
the YAML file later, like geometry angles or required
tags.

Dependencies:

* pip install beautifulsoup4
* pip install requests

## utils/xlstoyaml

Convert data models from XLSForms to YAML

Use this script for output a YAML data model that will be used in the
configuration files inside /validation directory.

#### Usage

```sh
python xls2yaml.py --category buildings > buildings.yaml
```

You may want to add more configuration parameters to the YAML file later.

## utils/poly2geojson

Converts .poly to .geojson 

#### Usage

```sh
python poly2geojson file.poly
```

## utils/clean-osmchanges.sql

You may want to run this query if you're not processing raw data
and you want to just produce statistics. 

This query deletes entries created from OsmChanges,
closed 1 day back or before , that has no corresponding Changeset.

We run this query every 24 hours for cleaning the database
when running the replicator process with areaFilter disabled 
for osmchanges (--osmnoboundary).

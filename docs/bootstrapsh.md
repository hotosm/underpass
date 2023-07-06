# Using the bootstrap.sh script

This script is prepared for a quick bootstrap of the Underpass database.

## Install requirements

```sh
pip install fiona
pip install shapely
```

## Quick start

Having Underpass installed with the database ready to use, run the script
passing region, country and DB username as arguments, for example:

```sh
    ./bootstrap.sh -r south-america -c ecuador -u underpass
```

This will:

1. Delete all entries into the database (WARNING: POTENTIAL LOSS OF DATA)
2. Download raw data from GeoFabrik
3. Run osm2psql for import it
4. Convert the country .poly file to .geojson and install config files
5. Run ./underpass --bootstrap for bootstrapping the validation table

### Script options

```
-r region (Region for bootstrapping)
   africa, antartica, asia, australia, central-america
   europe, north-america or south-america
-c country (Country inside the region)
-h host (Database host)
-u user (Database user)
-p port (Database port)
-d database (Database name)
-l (Use local files instead of download them)
```

Use the `-l` when you have your .pbf and .poly files already downloaded and
you don't want to download them again. The script will look for those files
using the `-r` and `-c` arguments, for example

```sh
    ./bootstrap.sh -r south-america -c ecuador -u underpass -l
```

Will look for these files:

```
argentina-latests.osm.pbf
argentina.poly
```

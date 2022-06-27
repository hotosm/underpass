## Initialize Rawdata Loading

Prepare your osm.pbf for loading before start. You can download it from different sources such as Geofabrik , Planet.
- Install [osm2pgsql v1.6.0](https://github.com/openstreetmap/osm2pgsql/releases/tag/1.6.0)
- Download and clone underpass
- Navigate to raw Directory 
- Download Planet [Here](https://planet.osm.org/pbf/) or Use Geofabrik Planet [Here](https://osm-internal.download.geofabrik.de/index.html) with full metadata (Tested with .pbf file)
- Run Following command  with your db credentials to terminal, ***--slim** mode will be important if you want to do replication later ( if you don't need to update your database remove --slim option ( this will consume more ram during loading) )*  You can read manual of osm2pgsql more [here](https://osm2pgsql.org/doc/manual.html#)

>    <font size="2">
     You can Export following system variables in order to avoid passing db configuration time to time ( Optional )

     PGHOST behaves the same as the host connection parameter.
     PGHOSTADDR behaves the same as the hostaddr connection parameter. This can be set instead of or in addition to PGHOST to avoid DNS lookup overhead.
     PGPORT behaves the same as the port connection parameter.
     PGDATABASE behaves the same as the dbname connection parameter.
     PGUSER behaves the same as the user connection parameter.
     PGPASSWORD behaves the same as the password connection parameter. Use of this environment variable is not recommended for security reasons, as some operating systems allow non-root users to see process environment variables via ps; instead consider using a password file (see Section 34.15).</font> 

Sample of Exporting Database Creadentials (Optional , You can directly pass it as command line arguments as well )
```
export PGHOST=localhost
export PGPORT=5432
export PGUSER=admin
export PGPASSWORD=admin
export PGDATABASE=postgres
```

##### Command 
```osm2pgsql --create -H localhost -U admin -P 5432 -d postgres -W --extra-attributes --slim --output=flex --style ./raw.lua yourdownloaded.osm.pbf ```

with exported DB Param : 

```osm2pgsql --create --extra-attributes --slim --output=flex --style ./raw.lua yourdownloaded.osm.pbf ```

> **Note:** It is tested with osm2pgsql 1.6.0 version only , If data is loaded without --slim mode you will have query ready tables nodes,ways_line,ways_poly and relations . When you use --slim mode it will store meta data to database itself . It will create three additional tables : planet_osm_nodes,planet_osm_ways, planet_osm_relations which will be used during update only , Rather than that you will be querying nodes,ways_line,ways_poly and relations table to get data !

## Prepare Your Tables

Install [psycopg2](https://pypi.org/project/psycopg2/) and [osmium](https://pypi.org/project/osmium/) in your python env

Create ***config.txt*** file in the directory and Put your credentials inside it like this

```

[RAW_DATA]

host=localhost

user=

password=

dbname=

port=

```

- **Create Basic Indexes** : 

    ```
    psql -h localhost -U admin -d postgres -a -f ./basic_indexes.sql
    ```

- **Create Grid Table** : 

    ```
    psql -h localhost -U admin -d postgres -a -f ./grid.sql
    ```

- **Apply Grid Update Script** :
    There is grid_update script , Which is responsible for the Grid Column update on the tables which will be null intially when you import 

    **Run Script**

    ```
    python grid_update
    ```


    >Script will take optional parameters if required , Default it will populate ways_poly from latest row date available on table to earliest row date with frequency of Daily
    
    Find More details with **help** 

    ```
    python grid_update -h
    ```

- **Create Geo Indexes & Cluster** :
    ```
    psql -h localhost -U admin -d postgres -a -f ./geo_indexes.sql
    ```



##  Initialize Update Script
  

Now run init , This will create replication status table in db

>Export database password or keep it inside systemd service or pass W after command   -- -W

```python rawdata-replication init ```

Replication script will use 'https://planet.openstreetmap.org/replication/minute'.

Now Run update with lua script file location : *-s* parameter like this (Considering you are inside raw data directory)


```
python rawdata-replication update -s raw.lua
```

with force password prompt (Only if you wish to supply pass from command) :

```
python rawdata-replication update -s raw.lua -- -W
```

Read more documentation [here](https://osm2pgsql.org/doc/manual.html#advanced-topics) 

## Configure Per Minute Replication

There are multiple options to run this python script per minute , You can either setup a cronjob or setup a systemd service

## Initialize Rawdata Loading

Prepare your osm.pbf for loading before start. You can download it from different sources such as Geofabrik , Planet.
- Install [osm2pgsql > v1.6.0](https://github.com/openstreetmap/osm2pgsql/releases/tag/1.6.0)
- Download and clone underpass
- Navigate to raw Directory 
- Download Planet pbf file[Here](https://planet.osm.org/pbf/) or Use Geofabrik Pbf file [Here](https://osm-internal.download.geofabrik.de/index.html) with full metadata (Tested with .pbf file) , you can pass this link to script itself when you run ```run.py```
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

## Prepare Your Tables

Install [psycopg2](https://pypi.org/project/psycopg2/), [osmium](https://pypi.org/project/osmium/) and [dateutil](https://pypi.org/project/python-dateutil/) , wget in your python env . You can install using ```requirements.txt``` too 

Create ***db_config.txt*** file in the directory and Put your credentials inside it like this

```

[RAW_DATA]

host=localhost

user=

password=

dbname=

port=

```

- **Start Application** :

    **Run Script**

    ```
    python run.py
    ```

    >Script will ask you few question about how you want to do it , it will ask for source , you can either pass download link or pass filepath where you have downloaded file 
    

##  Initialize Update Script
  

By default ```run.py``` will take care of replication but if you want to run it by yourself you can run this 

>Export database password or keep it inside systemd service or pass W after command   -- -W

```python replication init ```

Replication script will use 'https://planet.openstreetmap.org/replication/minute'.

Now Run update with lua script file location : *-s* parameter like this (Considering you are inside raw data directory)


```
python replication update -s raw.lua
```

with force password prompt (Only if you wish to supply pass from command) :

```
python replication update -s raw.lua -- -W
```

Read more documentation [here](https://osm2pgsql.org/doc/manual.html#advanced-topics) 

## Configure Per Minute Replication

There are multiple options to run this python script per minute , You can either setup a cronjob or setup a systemd service

## Initialize Rawdata Loading

Prepare your osm.pbf for loading before start. You can download it from different sources such as Geofabrik , Planet.
- Install [osm2pgsql v1.6.0](https://github.com/openstreetmap/osm2pgsql/releases/tag/1.6.0)
- Download and clone this underpass
- Navigate to raw Directory 
- Run Following command  with your db credentials to terminal, ***--slim** mode will be important if you want to do replication later* 
```osm2pgsql --create -H localhost -U admin -P 5432 -d postgres -W --extra-attributes --slim --output=flex --style ./raw.lua yourdownloaded.osm.pbf ```
> **Note:** It is tested with osm2pgsql 1.6.0 version only

##  Initialize Update Script

Install **psycopg2** and **osmium** in your python env
  

Create ***config.txt*** file in the directory and Put your credentials inside it like this

```

[RAW]

host=localhost

user=

password=

dbname=

port=

```

Now run init , This will create

  

```python rawdata-replication init ```

  

Now Run update with lua script file location : *-s* parameter like this (Considering you are inside raw data directory)

  

```python rawdata-replication update -s raw.lua```


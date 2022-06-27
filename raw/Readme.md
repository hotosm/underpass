## Initialize Rawdata Loading

Prepare your osm.pbf for loading before start. You can download it from different sources such as Geofabrik , Planet.
- Install [osm2pgsql v1.6.0](https://github.com/openstreetmap/osm2pgsql/releases/tag/1.6.0)
- Download and clone underpass
- Navigate to raw Directory 
- Run Following command  with your db credentials to terminal, ***--slim** mode will be important if you want to do replication later ( if you don't need to update your database remove --slim option ( this will consume more ram during loading) )*  You can read manual of osm2pgsql more [here](https://osm2pgsql.org/doc/manual.html#)

```osm2pgsql --create -H localhost -U admin -P 5432 -d postgres -W --extra-attributes --slim --output=flex --style ./raw.lua yourdownloaded.osm.pbf ```
> **Note:** It is tested with osm2pgsql 1.6.0 version only , If data is loaded without --slim mode you will have query ready tables nodes,ways_line,ways_poly and relations . When you use --slim mode it will store meta data to database itself . It will create three additional tables : planet_osm_nodes,planet_osm_ways, planet_osm_relations which will be used during update only , Rather than that you will be querying nodes,ways_line,ways_poly and relations table to get data !

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

Now run init , This will create replication status table in db

  

```python rawdata-replication init ```

  

Now Run update with lua script file location : *-s* parameter like this (Considering you are inside raw data directory)

  

```python rawdata-replication update -s raw.lua```


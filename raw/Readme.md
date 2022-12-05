## Initialize Rawdata Loading

- Install [osm2pgsql > v1.6.0](https://osm2pgsql.org/doc/install.html)
```
apt install osm2pgsql
```
- Download and clone underpass
- Navigate to raw Directory 

## Install Requirements
Install [psycopg2](https://pypi.org/project/psycopg2/), [osmium](https://pypi.org/project/osmium/) and [dateutil](https://pypi.org/project/python-dateutil/) , wget in your python env . You can install using ```requirements.txt``` too 

```
pip install -r requirements.txt
```


## Input Your DB Credentials 
Create ***db_config.txt*** file in the directory and Put your credentials inside it like this

```

[RAW_DATA]

host=localhost

user=

password=

dbname=

port=

```
## Start the Process
- **Run Script** :
    ```
    python run.py
    ```

    >Script will ask you few question about how you want to do it , it will ask for source , you can either pass download link or pass filepath where you have downloaded file . You can Download Planet pbf file[Here](https://planet.osm.org/pbf/) or Use Geofabrik Pbf file [Here](https://osm-internal.download.geofabrik.de/index.html) with full metadata (Tested with .pbf file) , you can pass this link to script itself
    

##  **OPTIONAL ** Run Update Script
  

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

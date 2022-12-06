## Getting Started 

- Install [osm2pgsql > v1.6.0](https://osm2pgsql.org/doc/install.html)
    ```
    apt install osm2pgsql
    ```
- Clone underpass and navigate to raw dir
    ```
    git clone https://github.com/hotosm/underpass.git && cd raw
    ```

- Install Requirements

    Install [psycopg2](https://pypi.org/project/psycopg2/), [osmium](https://pypi.org/project/osmium/) and [dateutil](https://pypi.org/project/python-dateutil/) , wget in your python env . You can install using ```requirements.txt``` too 

    ```
    pip install -r requirements.txt
    ```

 - Input Your DB Credentials 
 
    Create ***db_config.txt*** file in the directory and Put your credentials inside it like this

    ```
    [RAW_DATA]

    host=localhost
    user=
    password=
    dbname=
    port=
    ```
 - Start the Process

    Run.py Automates all the workflow / commands that you need 
    ```
    python run.py
    ```

    >Script will ask you few question about how you want to do it , it will ask for source , ask for whether you want to run replication or not . On source you can either pass download link or pass filepath where you have downloaded file . You can Download Planet pbf file[Here](https://planet.osm.org/pbf/) or Use Geofabrik Pbf file [Here](https://osm-internal.download.geofabrik.de/index.html) with full metadata (Tested with .pbf file) , you can pass this link to script itself . Follow example app_config json to know more

    If you are interested on Manual setup find Guide [here](./Manual.md) 
    

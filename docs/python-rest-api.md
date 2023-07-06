# Install & run the Underpass Python REST API

## Install requirements

```sh
pip install fastapi
pip install uvicorn
pip install psycopg2
```

## Setup

Check this config file for your database configuration:

[python/restapi/config.py](https://github.com/hotosm/underpass/blob/master/python/restapi/config.py)

## Run

```sh
uvicorn main:app --reload
```

Or, if you want to make the service available across the network:

```sh
uvicorn main:app --reload --host 0.0.0.0
```

Now you'll be able to open `http://localhost:8000` and see a welcome message.


## Run a RESTful API

### Setup & run

#### Install requirements

```sh
cd python/restapi/ && pip install -r requirements.txt
```

#### Setup DB connection

Set the database connection string as an environment variable:

```sh
export UNDERPASS_API_DB=postgresql://localhost/underpass
```

#### Run

```sh
uvicorn main:app --reload 
```

### Making queries

#### Raw data

#### Get polygons

```sh
curl http://localhost:8000/raw/polygons -X POST \
    -H 'content-type: application/json' \
    --data-raw '{"area":"-180 90,180 90, 180 -90, -180 -90,-180 90", "tags": "building=yes"}'
```

#### Get lines

```sh
curl http://localhost:8000/raw/lines -X POST \
    -H 'content-type: application/json' \
    --data-raw '{"area":"-180 90,180 90, 180 -90, -180 -90,-180 90", "tags": "highway"}'
```

#### Get nodes

```sh
curl http://localhost:8000/raw/nodes -X POST \
    -H 'content-type: application/json' \
    --data-raw '{"area":"-180 90,180 90, 180 -90, -180 -90,-180 90", "tags": "amenity"}'
```

#### Get all together (polygons, lines and nodes)

```sh
curl http://localhost:8000/raw/all -X POST \
    -H 'content-type: application/json' \
    --data-raw '{"area":"-180 90,180 90, 180 -90, -180 -90,-180 90", "tags": "building"}'
```

#### Get list of polygons

```sh
curl http://localhost:8000/raw/polygonsList -X POST \
    -H 'content-type: application/json' \
    --data-raw '{"area":"-180 90,180 90, 180 -90, -180 -90,-180 90", "tags": "building=yes"}'
```

#### Get list of lines

```sh
curl http://localhost:8000/raw/linesList -X POST \
    -H 'content-type: application/json' \
    --data-raw '{"area":"-180 90,180 90, 180 -90, -180 -90,-180 90", "tags": "highway"}'
```

#### Get list of nodes

```sh
curl http://localhost:8000/raw/nodesList -X POST \
    -H 'content-type: application/json' \
    --data-raw '{"area":"-180 90,180 90, 180 -90, -180 -90,-180 90", "tags": "amenity"}'
```

#### Get list of all together (polygons, lines and nodes)

```sh
curl http://localhost:8000/raw/allList -X POST \
    -H 'content-type: application/json' \
    --data-raw '{"area":"-180 90,180 90, 180 -90, -180 -90,-180 90", "tags": "building"}'
```

### Get data quality reports in CSV or GeoJSON

#### Get report for geometries

```sh
curl http://localhost:8000/report/dataQualityGeo -X POST \
    -H 'content-type: application/json' \
    --data-raw '{"fromDate":"2022-12-28T00:00:00", "hashtags": "hotosm"}'
```

In CSV format instead of GeoJSON:

```sh
curl http://localhost:8000/report/dataQualityGeo/csv -X POST \
    -H 'content-type: application/json' \
    --data-raw '{"fromDate":"2022-12-28T00:00:00", "hashtags": "hotosm"}'
```

#### Get report for tags

```sh
curl http://localhost:8000/report/dataQualityTags -X POST \
    -H 'content-type: application/json' \
    --data-raw '{"fromDate":"2022-12-28T00:00:00", "hashtags": "hotosm"}'
```

#### Get statistics for tags

```sh
curl http://localhost:8000/report/dataQualityTagStats -X POST \
    -H 'content-type: application/json' \
    --data-raw '{"fromDate":"2022-12-28T00:00:00", "hashtags": "hotosm"}'
```

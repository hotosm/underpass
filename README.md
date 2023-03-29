![CI Build and Testing](https://github.com/hotosm/underpass/actions/workflows/run_tests.yml/badge.svg)
![Doxygen](https://github.com/hotosm/underpass/actions/workflows/main.yml/badge.svg)

# Underpass

Underpass is a **data analysis engine** that process OpenStreetMap data
and provides customizable **statistics and validation** reports in **near real time**.

## Quick start

```
    git clone https://github.com/hotosm/underpass.git
    sh docker/install.sh
```

After installation is done, a process will start downloading and processing
OSM data from a week ago, storing the results on the database and keep running
for updating data every minute.

You can start/stop the replication process:

sh docker/services-start.sh
sh docker/services-stop.sh

If you want to avoid using Docker and build Underpass on your system, check
the [install](https://github.com/hotosm/underpass/blob/master/doc/install.md) 
documentation.

## Using the data

### Reports on your browser

Open http://127.0.0.1:5000 on your browser and you'll see a set of UI components
for OSM data analysis.

### REST API

You can request the REST API directly:

```
curl --location 'http://127.0.0.1:8000/report/dataQualityTag/csv' \
--header 'Content-Type: application/json' \
--data '{
    "fromDate": "2023-01-01T00:00:00",
    "hashtags": []
}'
```

### DB API

See an example of how generate reports using the DB API:

docker exec -w /code/util/python/dbapi/example -t underpass \
> python csv-report.py

### Python API

Use the Python example for download and analyze Changeset:

docker exec -w /code/util/python/examples -t underpass \
python validation.py -u https://www.openstreetmap.org/api/0.6/changeset/133637588/download -c place

# Product roadmap

We have included below a reference to the Underpass Product Roadmap [subject to change]. 
We hope it is a useful reference for anyone wanting to get involved.

![image](https://user-images.githubusercontent.com/98902727/218773383-6c56e45d-132a-43d3-9fa9-ddff94c89b7c.png)

# Core documentation

Check the [docs](https://hotosm.github.io/underpass/index.html) for
internal documentation of all the C++ classes.


## Get data from the Underpass DB using Python

### Connect to the database

```py
from api.db import UnderpassDB`
db = UnderpassDB("postgresql://localhost/underpass")
db.connect()
```

### Get raw data 

```py
from api import raw
rawer = raw.Raw(db)
```

#### Get polygons

```py
polygons = rawer.getPolygons( 
    area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    tags = "building=yes",
    hashtag = "",
    dateFrom = "",
    dateTo = "",
    page = 0
)
```

#### Get lines

```py
lines = rawer.getLines( 
    area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    tags = "highway",
    hashtag = "",
    dateFrom = "",
    dateTo = "",
    page = 0
)
```


#### Get nodes

```py
nodes = rawer.getNodes( 
    area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    tags = "amenity",
    hashtag = "",
    dateFrom = "",
    dateTo = "",
    page = 0
)
```

#### Get all together (polygons, lines and nodes)

```py
nodes = rawer.getAll( 
    area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    tags = "building",
    hashtag = "",
    dateFrom = "",
    dateTo = "",
    page = 0
)
```

#### Get list of polygons

```py
polygons = rawer.getPolygonsList( 
    area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    tags = "building",
    hashtag = "",
    dateFrom = "",
    dateTo = "",
    page = 0
)
```

#### Get list of lines

```py
lines = rawer.getLinesList( 
    area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    tags = "building",
    hashtag = "",
    dateFrom = "",
    dateTo = "",
    page = 0
)
```

#### Get list of nodes

```py
nodes = rawer.getNodesList( 
    area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    tags = "building",
    hashtag = "",
    dateFrom = "",
    dateTo = "",
    page = 0
)
```

#### Get list of all together (polygons, lines and nodes)

```py
all = rawer.getAllList( 
    area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    tags = "building",
    hashtag = "",
    dateFrom = "",
    dateTo = "",
    page = 0
)
```

### Get data quality reports in CSV or GeoJSON

```py
from api import report
reporter = report.Report(db)
```

#### Get report for geometries

```py
results = reporter.getDataQualityGeo(
    fromDate = "2022-12-28T00:00:00", 
    hashtags = ["hotosm"],
    responseType = "csv"
)
```

#### Get latest results for geometries

```py
results = reporter.getDataQualityGeoLatest()
```

For getting results in CSV format, instead of GeoJSON

```py
results = reporter.getDataQualityGeoLatest(
    responseType = "csv"
)
```

#### Get report for tags

```py
results = reporter.getDataQualityTag(
    fromDate = "2022-12-28T00:00:00", 
    hashtags = ["hotosm"],
    responseType = "csv"
)
```

#### Get statistics for tags

```py
results = reporter.getDataQualityTagStats(
    fromDate = "2022-12-28T00:00:00", 
    hashtags = ["hotosm"],
    responseType = "csv"
)
```
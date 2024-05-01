from enum import Enum

# DB table names
class Table(Enum):
    nodes = "nodes"
    lines = "ways_line"
    polygons = "ways_poly"
    relations = "relations"

# Geometry types
class GeoType(Enum):
    polygons = "Polygon"
    lines = "LineString"
    nodes = "Node"
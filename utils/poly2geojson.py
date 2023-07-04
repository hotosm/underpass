#!/usr/bin/env python3
#
# Copyright (c) 2023 Humanitarian OpenStreetMap Team
#
# This file is part of Underpass.
#
#     Underpass is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     Underpass is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with Underpass.  If not, see <https://www.gnu.org/licenses/>.

'''
    Converts .poly to .geojson 

    Usage: python poly2geojson file.poly
'''
import re
import argparse
import os
import fiona
from shapely.geometry import MultiPolygon, Polygon, mapping
import functools


def remove_file(file_name):
    try:
        os.remove(file_name)
    except OSError:
        pass


def read_polygon(polygon_filename):
    with open(polygon_filename) as f:
        return f.readlines()


def clean_poylgon(polygon_data):
    coordinates = polygon_data[2:][:-2]

    coordinates = [re.split(r'[\s\t]+', item) for item in coordinates]
    coordinates = [list(filter(None, item)) for item in coordinates]

    coordinates = functools.reduce(lambda a,b: a[-1].pop(0) and a if len(a[-1]) == 1 and a[-1][0] == 'END' else a.append(['END']) or a if b[0].startswith('END') else a[-1].append(b) or a, [[[]]] + coordinates)

    coordinates = [[(float(item[0]), float(item[1])) for item in coordgroup] for coordgroup in coordinates]

    return coordinates


def write_geojson(data, polygon_filename):
    geojson_filename = '.'.join(polygon_filename.split('.')[:-1]) + ".geojson"
    remove_file(geojson_filename)

    schema = {'geometry': 'MultiPolygon', 'properties': {}}

    with fiona.open(geojson_filename, 'w', 'GeoJSON', schema) as output:
        for elem in data:
            output.write({'geometry': mapping(MultiPolygon([Polygon(elem)])), 'properties': {}})


def main(polygon_filename):
    polygon_data = read_polygon(polygon_filename)
    coordinates = clean_poylgon(polygon_data)
    write_geojson(coordinates, polygon_filename)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("polygon_filename")
    args = parser.parse_args()

    main(args.polygon_filename)

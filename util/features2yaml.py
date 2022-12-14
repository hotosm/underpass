#!/usr/bin/python3
#
# Copyright (c) 2020, 2021, 2022 Humanitarian OpenStreetMap Team
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
    Get tags from the Map Features OSM wiki and output YAML

    Use this script for output a YAML data model
    that will be used in the configuration files
    inside /validation directory.

    Usage:

    python features2yaml.py --category buildings > buildings.yaml
    python features2yaml.py --key building:material > building:material.yaml
    python features2yaml.py --url https://wiki.openstreetmap.org/wiki/Key:landuse
    python features2yaml.py --f ../../place.html

    You may want to add more configuration parameters to
    the YAML file later, like geometry angles or required
    tags.

    Dependencies:

    pip install beautifulsoup4
    pip install requests

'''

import requests
import argparse
from bs4 import BeautifulSoup

def read_source(category, default_key, url, file):
    if category:
        url = "https://wiki.openstreetmap.org/wiki/" + category
        return requests.get(url).content
    elif default_key:
        url = "https://wiki.openstreetmap.org/wiki/Key:" + default_key
        return requests.get(url).content
    elif url:
        return requests.get(url).content
    f = open(file,"r")
    lines = f.readlines()
    return ''.join(lines)

def features2yaml(category, default_key, url, file):
    content = read_source(category, default_key, url, file)
    soup = BeautifulSoup(content, 'html.parser')
    tables = soup.find_all('table', class_='wikitable')
    last_key = ""
    if default_key:
        value_index = 0
    else:
        value_index = 1
    print("tags:")
    for table in tables:
        rows = table.find_all("tr")
        for row in rows:
            columns = row.find_all("td")
            if len(columns) > 0:
                if not default_key:
                    key = columns[0].text.replace(" ", "").replace("\n", "")
                else:
                    key = default_key
                if key:
                    if last_key != key:
                        last_key = key
                        print("  - " + key + ":")
                    value = columns[value_index].text.replace(" ", "").replace("\n", "")
                    if value and value != "userdefined" and (value_index < len(value)) and value[value_index] != "<" and value[-1] != ">":
                        if value.find("|") > -1:
                            values = value.split("|")
                        elif value.find(",") > -1:
                            values = value.split(",")
                        else:
                            values = [value]
                        for val in values:
                            print("    - " + val)

def main():
    args = argparse.ArgumentParser()
    args.add_argument("--category", "-c", help="Category", type=str, default=None)
    args.add_argument("--key", "-k", help="Key", type=str, default=None)
    args.add_argument("--url", "-u", help="Url", type=str, default=None)
    args.add_argument("--file", "-f", help="File", type=str, default=None)
    args = args.parse_args()
    if args.category or args.url or args.key or args.file:
        features2yaml(args.category, args.key, args.url, args.file)
    else:
        print("Usage: python features2yaml.py --category buildings > buildings.yaml")

if __name__ == "__main__":
    main()
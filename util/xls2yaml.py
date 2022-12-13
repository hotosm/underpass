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
    Convert data models from XLSForms to YAML

    Use this script for output a YAML data model
    that will be used in the configuration files
    inside /validation directory.

    Usage:
    python xls2yaml.py --category buildings > buildings.yaml

    You may want to add more configuration parameters to
    the YAML file later, like geometry angles or required
    tags.

'''

import pandas as pd
import argparse

ignored_keys = ["yes_no", "nan", "model", "category"]

def read_file(path, columns):
    return pd.read_excel(
        path,
        sheet_name="choices",
        usecols=columns
    )

def xml2yaml(category, path, columns):
    if category:
        df = read_file("https://github.com/hotosm/odkconvert/blob/main/XForms/" + category + ".xls?raw=true", columns)
    else:
        df = read_file(path, columns)
    print("tags:")
    last_tag = ''
    key_column = columns[0]
    tag_column = columns[1]
    for index, row in df.iterrows():
        if str(row[columns[0]]) not in ignored_keys:
            if last_tag != row[key_column]:
                last_tag = row[key_column]
                print("  - " + str(row[key_column]) + ":")
            print("    - " + str(row[tag_column]))

def main():
    args = argparse.ArgumentParser()
    args.add_argument("--category", "-c", help="Category", type=str, default=None)
    args.add_argument("--path", "-p", help="Path", type=str, default=None)
    args.add_argument("--columns", help="Columns", type=str, default="list_name,name")
    args = args.parse_args()
    if args.category or args.path:
        xml2yaml(args.category, args.path, args.columns.split(","))
    else:
        print("Usage: python xls2yaml.py --category buildings > buildings.yaml")

if __name__ == "__main__":
    main()

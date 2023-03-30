#!/usr/bin/python3
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

#     This is an example on how you can use the Underpass C++ Engine
#     trough the Underpass Python package

import underpass as u
import argparse
import requests

def validateOsmChange(xml: str, check: str):
    validator = u.Validate()
    return validator.checkOsmChange(xml, check)

def main():
    args = argparse.ArgumentParser()
    args.add_argument("--file", "-f", help="OsmChange file", type=str, default=None)
    args.add_argument("--url", "-u", help="OsmChange URL", type=str, default=None)
    args.add_argument("--check", "-c", help="Check (ex: building)", type=str, default=None)
    args = args.parse_args()

    if args.check:
        data = None
        if args.file:
            with open(args.file, 'r') as file:
                data = file.read().rstrip()
        elif args.url:
            r = requests.get(args.url)
            data = r.text
        print(validateOsmChange(data, args.check))
    else:
        print("Usage: python validation.py -f <file> -c <check>")
        print("Example: python validation.py -f ../../../testsuite/testdata/validation/building-tag.osc -c building")
if __name__ == "__main__":
    main()

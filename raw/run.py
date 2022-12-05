#!/usr/bin/env python3

# Copyright (c) 2020, 2021 Humanitarian OpenStreetMap Team
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
#     along with Underpass.  If not, see <https:#www.gnu.org/licenses/>.
import json
import os
import subprocess
import time
from configparser import ConfigParser
from os.path import exists
from urllib.parse import urlparse

import wget

db_config = ConfigParser()
db_config.read("db_config.txt")

os.environ["PGHOST"] = db_config.get("RAW_DATA", "host", fallback="localhost")
os.environ["PGPORT"] = db_config.get("RAW_DATA", "port", fallback="5432")
os.environ["PGUSER"] = db_config.get("RAW_DATA", "user", fallback="postgres")
os.environ["PGPASSWORD"] = db_config.get("RAW_DATA", "password", fallback="postgres")
os.environ["PGDATABASE"] = db_config.get("RAW_DATA", "dbname", fallback="postgres")


def is_local(url):
    url_parsed = urlparse(url)
    if url_parsed.scheme in ("file", ""):  # Possibly a local file
        return exists(url_parsed.path)
    return False


def run_subprocess_cmd(cmd):
    try:
        subprocess.check_output(cmd, env=os.environ)
    except subprocess.CalledProcessError as e:
        print(e.output)
        raise e.output


def save_config(config):
    # save my config
    with open("app_config.json", "w") as f:
        f.write(json.dumps(config))


if not exists("app_config.json"):

    config = {
        "pbf2db_insert": False,
        "pre_index": False,
        "db_operation": {
            "create": {"grid": False, "country": False},
            "update": {
                "nodes": False,
                "ways_line": False,
                "ways_poly": {"country": False, "grid": False},
                "relations": False,
            },
        },
        "post_index": False,
        "run_replication": False,
        "replication_init": False,
        "replication": {},
    }

    source_url = input(
        "Data Source ? Paste link to Download or Downloaded file path ( .pbf file ) : \n"
    )
    config["source"] = source_url
    do_replciation = input(
        "Do you want to run replication later on ? Default : NO . Type y/yes for yes : \n"
    )
    if do_replciation.lower() == "y" or do_replciation.lower() == "yes":
        config["run_replication"] = True
        print("Will Run Replication later on")

    save_config(config)

with open("app_config.json", "r") as openfile:
    # Reading from json file
    config = json.load(openfile)

source_path = config["source"]
if not is_local(config["source"]):
    if not exists("data"):
        os.mkdir("data")
    source_path = "data/source.osm.pbf"
    if not exists(source_path):
        print(f"Starting download for : {source_path}")

        response = wget.download(config["source"], source_path)

if not config["pbf2db_insert"]:
    print(f"Starting Import Process (1/10)... \n")

    osm2pgsql = [
        "osm2pgsql",
        "--create",
        "--slim",
        "--drop",
        "--extra-attributes",
        "--output=flex",
        "--style",
        "./raw.lua",
        source_path,
    ]
    if config["run_replication"]:
        osm2pgsql = [
            "osm2pgsql",
            "--create",
            "--slim",
            "--extra-attributes",
            "--output=flex",
            "--style",
            "./raw.lua",
            source_path,
        ]
    run_subprocess_cmd(osm2pgsql)
    config["pbf2db_insert"] = True

# check point
save_config(config)


if not config["pre_index"]:
    ## build pre indexes
    print(f"Building Pre Indexes(2/10) ... \n")

    basic_index_cmd = ["psql", "-a", "-f", "sql/pre_indexes.sql"]
    run_subprocess_cmd(basic_index_cmd)
    config["pre_index"] = True

## create grid table
if not config["db_operation"]["create"]["grid"]:
    print(f"Creating  Grid Table (3/10) ... \n")

    grid_insert = ["psql", "-a", "-f", "sql/grid.sql"]
    run_subprocess_cmd(grid_insert)
    config["db_operation"]["create"]["grid"] = True
if not config["db_operation"]["create"]["country"]:
    print(f"Creating  Country Table (4/10) ... \n")

    ## create country table
    country_table = ["psql", "-a", "-f", "sql/countries_un.sql"]
    run_subprocess_cmd(country_table)
    config["db_operation"]["create"]["country"] = True

if not config["db_operation"]["update"]["nodes"]:
    print(f"Updating  Nodes:Country Table (5/10) ... \n")
    ## initiate country update for nodes
    field_update_cmd = [
        "python",
        "field_update",
        "-target_table",
        "nodes",
        "--target_column",
        "country",
        "--target_geom",
        "geom",
        "--source_table",
        "countries_un",
        "--source_column",
        "ogc_fid",
        "--source_geom",
        "wkb_geometry",
    ]
    run_subprocess_cmd(field_update_cmd)
    config["db_operation"]["update"]["nodes"] = True

if not config["db_operation"]["update"]["ways_poly"]["country"]:
    print(f"Updating  ways_poly:Country Table (6/10) ... \n")

    ## initiate country update for ways_poly
    field_update_cmd = [
        "python",
        "field_update",
        "--target_table",
        "ways_poly",
        "--target_column",
        "country",
        "--target_geom",
        "geom",
        "--source_table",
        "countries_un",
        "--source_column",
        "ogc_fid",
        "--source_geom",
        "wkb_geometry",
    ]
    run_subprocess_cmd(field_update_cmd)
    config["db_operation"]["update"]["ways_poly"]["country"] = True

if not config["db_operation"]["update"]["ways_poly"]["grid"]:
    print(f"Updating  ways_poly:grid Table (7/10) ... \n")

    # grid update for ways_poly
    field_update_cmd = [
        "python",
        "field_update",
        "-target_table",
        "ways_poly",
        "--target_column",
        "grid",
        "--target_geom",
        "geom",
        "--source_table",
        "grid",
        "--source_column",
        "poly_id",
        "--source_geom",
        "geom",
        "--type",
        "int",
    ]
    run_subprocess_cmd(field_update_cmd)
    config["db_operation"]["update"]["ways_poly"]["grid"] = True

if not config["db_operation"]["update"]["ways_line"]:
    print(f"Updating  ways_line:country Table (8/10) ... \n")

    ## initiate country update for ways_line
    field_update_cmd = [
        "python",
        "field_update",
        "-target_table",
        "ways_line",
        "--target_column",
        "country",
        "--target_geom",
        "geom",
        "--source_table",
        "countries_un",
        "--source_column",
        "ogc_fid",
        "--source_geom",
        "wkb_geometry",
    ]
    run_subprocess_cmd(field_update_cmd)
    config["db_operation"]["update"]["ways_line"] = True

if not config["db_operation"]["update"]["relations"]:
    print(f"Updating  relations:country Table (9/10) ... \n")

    ## initiate country update for relations
    field_update_cmd = [
        "python",
        "field_update",
        "-target_table",
        "relations",
        "--target_column",
        "country",
        "--target_geom",
        "geom",
        "--source_table",
        "countries_un",
        "--source_column",
        "ogc_fid",
        "--source_geom",
        "wkb_geometry",
    ]
    run_subprocess_cmd(field_update_cmd)
    config["db_operation"]["update"]["relations"] = True

if not config["post_index"]:
    print(f"Building  Post Indexes (10/10) ... \n")

    ## build post indexes
    basic_index_cmd = ["psql", "-a", "-f", "sql/post_indexes.sql"]
    run_subprocess_cmd(basic_index_cmd)
    config["post_index"] = True

# checkpoint
save_config(config)


if config["run_replication"]:

    if not config["replication_init"]:
        ## db is ready now intit replication
        print("\nRunning Replication init .. \n")

        replication_init = ["python", "replication", "init"]
        run_subprocess_cmd(replication_init)
        config["replication_init"] = True

    if "country" not in config["replication"]:
        country_filter = input(
            "\nBy default replication will cover whole world , If you have loaded country do you want to keep only your country data ? y/yes to yes :\n"
        )
        if country_filter.lower() == "y" or country_filter.lower() == "yes":
            coutry_list = input(
                "Enter your country ogc_fid from countries_un table in database ( If multiple countries split by ,): \n"
            )
            config["replication"]["country"] = [int(x) for x in coutry_list.split(",")]

# checkpoint
save_config(config)


if "replication" in config:
    print(f"Starting  Replication ... \n")

    while True:  # run replication forever
        # --max-diff-size 10 mb as default
        start = time.time()
        replication_cmd = [
            "python",
            "replication",
            "update",
            "-s",
            "raw.lua",
            "--max-diff-size",
            "10",
        ]
        run_subprocess_cmd(replication_cmd)
        if (time.time() - start) < 60:
            time.sleep(60)

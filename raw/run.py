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
        "replication_init": False,
    }

    source_url = input("Source to Download ( .pbf file ) ")
    config["source"] = source_url
    do_replciation = input(
        "Do you want to run replication later on ? Default : no . Type y/yes for yes "
    )
    if do_replciation.lower() == "y" or "yes":
        config["replication_init"] = True

    with open("app_config.json", "w") as outfile:
        json.dump(config, outfile)

with open("app_config.json", "r") as openfile:
    # Reading from json file
    config = json.load(openfile)

source_path = config["source"]
if not is_local(config["source"]):
    source_path = "data/source.osm.pbf"
    if not exists(source_path):
        response = wget.download(config["source"], "source.osm.pbf")

if not config["pbf2db_insert"]:
    osm2pgsql = [
        "osm2pgsql",
        "--create",
        "--slim",
        "--drop",
        "--extra-attributes",
        "--output=flex",
        "--style",
        "./raw.lua",
        "./source.osm.pbf",
    ]
    if config["replication_init"]:
        osm2pgsql = [
            "osm2pgsql",
            "--create",
            "--slim",
            "--extra-attributes",
            "--output=flex",
            "--style",
            "./raw.lua",
            "./source.osm.pbf",
        ]
    run_subprocess_cmd(osm2pgsql)
    config["pbf2db_insert"] = True

if not config["pre_index"]:
    ## build pre indexes
    basic_index_cmd = ["psql", "-a", "-f", "sql/pre_indexes.sql"]
    run_subprocess_cmd(basic_index_cmd)
    config["pre_index"] = True

## create grid table
if not config["db_operation"]["create"]["grid"]:
    grid_insert = ["psql", "-a", "-f", "sql/grid.sql"]
    run_subprocess_cmd(grid_insert)
    config["db_operation"]["create"]["grid"] = True
if not config["db_operation"]["create"]["country"]:

    ## create country table
    country_table = ["psql", "-a", "-f", "sql/countries_un.sql"]
    run_subprocess_cmd(country_table)
    config["db_operation"]["create"]["country"] = True

if not config["db_operation"]["update"]["nodes"]:

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
    run_subprocess_cmd(field_update_cmd, check=True)
    config["db_operation"]["update"]["nodes"] = True

if not config["db_operation"]["update"]["ways_poly"]["country"]:
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

    ## build post indexes
    basic_index_cmd = ["psql", "-a", "-f", "sql/post_indexes.sql"]
    run_subprocess_cmd(basic_index_cmd)
    config["post_index"] = True

if not config["replication_init"]:
    ## db is ready now intit replication

    replication_init = ["python", "replication", "init"]
    run_subprocess_cmd(replication_init)
    config["replication_init"] = True

if "replication" not in config:
    replication_answer = input("Do you want to Run Replication ? , y/yes for yes")
    if replication_answer.lower() == "y" or "yes":
        config["replication"] = {}
        country_filter = input(
            "By default replication will cover whole world , If you have loaded country do you want to keep only your country data ?"
        )
        if country_filter.lower() == "y" or "yes":
            coutry_list = input(
                "Enter your country ogc_fid from countries_un table in database ( If multiple countries split by ,): "
            )
            config["replication"]["country"] = list(coutry_list)


if "replication" in config:
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

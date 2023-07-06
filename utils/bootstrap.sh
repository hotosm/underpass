#!/bin/bash
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


#    -----
#    This is a utility script for bootstrapping an Underpass
#    database with a full country OSM data.
#    -----

localfiles='false'

while getopts r:c:h::u:p:d:l flag
do
    case "${flag}" in
        r) region=${OPTARG};;
        c) country=${OPTARG};;
        h) host=${OPTARG};;
        u) user=${OPTARG};;
        p) port=${OPTARG};;
        d) database=${OPTARG};;
        l) localfiles=${OPTARG};;
    esac
done

REGION=${region}
COUNTRY=${country}
HOST=${host:-localhost}
PORT=${port:-5432}
DB=${database:-underpass}

if [ -n "${user}" ] 
then
    USER=${user}
else
    USER=$(whoami)
fi

if [ -n "${REGION}" ] && [ -n "${COUNTRY}" ] 
then

    echo Region: $REGION
    echo Country: $COUNTRY
    echo Host: $HOST
    echo Username: $USER
    echo Port: $PORT
    echo Database: $DB

    if [ -z "${localfiles}" ]
    then
        echo "Use local files?: yes"
    fi

    echo " "
    echo "*** WARNING: THIS ACTION WILL OVERWRITE DATA IN THE CURRENT DATABASE ***"
    echo " "
    read -p "Are you sure? Y/N" -n 1 -r
    echo " "
    if [[ $REPLY =~ ^[Yy]$ ]]
    then

        read -s -p "Enter your database password: " -r
        PASS=$REPLY
        echo " "
        echo "Cleaning database ..."
        PGPASSWORD=$PASS psql --host $HOST --user $USER --port $PORT $DB -c 'delete from raw_poly; delete from raw_node; delete from validation; delete from changesets'

        if [ -z "${localfiles}" ]
        then
            echo "(Using local files)"
        else
            echo "Downloading updated map data ..." && \
            wget https://download.geofabrik.de/$REGION/$COUNTRY-latest.osm.pbf && \
            wget https://download.geofabrik.de/$REGION/$COUNTRY.poly
        fi

        echo "Importing data (this will take some time) ..."
        PGPASSWORD=$PASS osm2pgsql -H $HOST -U $USER -P $PORT -d $DB --extra-attributes --output=flex --style ./raw-underpass.lua $COUNTRY-latest.osm.pbf
        PGPASSWORD=$PASS psql --host $HOST --user $USER --port $PORT $DB < raw-underpass.sql

        echo "Configuring Underpass ..."
        python3 poly2geojson.py $COUNTRY.poly && \
        cp $COUNTRY.geojson ../config/priority.geojson && \
        cd ../build && \
        make install && \
        echo "Bootstrapping validation database ..."
        ./underpass --bootstrap
        echo "Done."
        echo " "
        echo "Now you can run the Underpass replicator for keeping the"
        echo "database up-to-date"
    fi

else
    echo "This is a utility script for bootstrapping an Underpass"
    echo "database with a full country OSM data."
    echo " "
    echo "Usage: sh bootstrap.sh -r <region> -c <country>"
    echo "Example: sh bootstrap.sh -r asia -c nepal"
    echo " "
    echo "[Options]"
    echo " -r region (Region for bootstrapping)"
    echo "  africa, antartica, asia, australia, central-america,"
    echo "  europe, north-america or south-america"
    echo " -c country (Country inside the region)"
    echo " -h host (Database host)"
    echo " -u user (Database user)"
    echo " -p port (Database port)"
    echo " -d database (Database name)"
    echo " -l (Use local files instead of download them)"
fi

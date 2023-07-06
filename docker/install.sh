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
#    This is a utility script for installing and bootstrapping Underpass
#    using Docker
#    -----


while getopts r:c: flag
do
    case "${flag}" in
        r) region=${OPTARG};;
        c) country=${OPTARG};;
    esac
done

REGION=${region:-asia}
COUNTRY=${country:-nepal}

if [ -n "${REGION}" ] && [ -n "${COUNTRY}" ] 
then

    echo " "
    echo "You are about to install and run Underpass with the following configuration:"
    echo " "
    echo Region: $REGION
    echo Country: $COUNTRY
    echo " "
    echo "This will take some time."
    echo " "
    read -p "Are you sure? Y/N " -n 1 -r
    echo " "
    if [[ $REPLY =~ ^[Yy]$ ]]
    then
        echo " "
        echo "Ok, grab a coffee :)" && \
        echo " "
        echo "Composing containers ..." && \
        docker-compose up -d && \
        docker exec -t underpass cp /code/docker/underpass-config.yaml /root/.underpass && \
        echo "Building Underpass ..." && \
        docker exec -t underpass mkdir -p /usr/local/lib/underpass/ && \
        docker exec -w /code -t underpass ./autogen.sh && \
        docker exec -w /code -t underpass mkdir -p /code/build && \
        docker exec -w /code/build -t underpass ../configure --enable-shared && \
        docker exec -w /code/build -t underpass make -j $(nproc) && \
        docker exec -t underpass ln -s /usr/bin/python3 /usr/bin/python && \
        echo "Installing Postgres ..." && \
        docker exec -t underpass apt update && \
        docker exec -t underpass apt -y install postgresql postgresql-contrib && \
        echo "Creating database ..." && \
        docker exec -t underpass psql -U underpass -c 'create database underpass';
        echo "Setting up database ..." && \
        docker exec -w /code/setup -t underpass psql -U underpass underpass -f underpass.sql && \
        echo "Setting up config ..." && \
        docker exec -t underpass cp /code/docker/underpass-config.yaml /root/.underpass && \
        echo "Setting up utils ..." && \
        docker exec -t underpass apt -y install gunicorn && \
        docker exec -t underpass apt -y install nodejs npm && \
        docker exec -t underpass npm cache clean -f && \
        docker exec -t underpass npm install -g n && \
        docker exec -t underpass n stable && \
        docker exec -t underpass npm install --global yarn && \
        docker exec -w /code/js -t underpass yarn install && \
        docker exec -t underpass apt -y install tmux && \
        docker exec -t underpass apt -y install python3-pip && \
        docker exec -t underpass pip install requests && \
        docker exec -t underpass pip install psycopg2 && \
        docker exec -t underpass pip install uvicorn && \
        docker exec -t underpass pip install fastapi && \
        docker exec -t underpass pip install fiona && \
        docker exec -t underpass pip install shapely && \
        docker exec -t underpass apt -y install osm2pgsql && \
        echo "Downloading bootstrap sample data ($REGION/$COUNTRY) ..." && \
        docker exec -w /code/utils -t underpass wget https://download.geofabrik.de/$REGION/$COUNTRY-latest.osm.pbf && \
        docker exec -w /code/utils -t underpass wget https://download.geofabrik.de/$REGION/$COUNTRY.poly && \
        docker exec -w /code/utils -t underpass python3 poly2geojson.py $COUNTRY.poly && \
        docker exec -w /code/utils -t underpass cp $COUNTRY.geojson ../config/priority.geojson && \
        docker exec -w /code/build -t underpass make install && \
        docker exec -w /code/utils -t underpass osm2pgsql -H postgis -U underpass -p underpass -P 5432 -d underpass --extra-attributes --output=flex --style ./raw-underpass.lua $COUNTRY-latest.osm.pbf && \
        docker exec -w /code/utils -t underpass psql -h postgis -p 5432 -w underpass underpass -f raw-underpass.sql && \
        docker exec -w /code/build -t underpass ./underpass --bootstrap && \
        echo "Starting replicator service ..." && \
        docker exec -t underpass tmux new-session -d -s replicator 'cd /code/build && ./underpass -t $(date +%Y-%m-%dT%H:%M:%S -d "2 days ago")' && \
        docker exec -t underpass tmux new-session -d -s rest-api 'cd /code/python/restapi && uvicorn main:app --reload --host 0.0.0.0' && \
        docker exec -t underpass tmux new-session -d -s react-cosmos 'cd /code/js && echo export const center=[$(psql -f ../utils/randombuildingcentroid.sql | grep ,)] > src/fixtures/center.js && yarn cosmos' && \
        echo "Done! 🚀" && \
        echo "Underpass is now running on a Docker container" && \
        echo "---" && \
        echo "Check your browser: http://localhost:5000 " && \
        echo "REST API: http://localhost:8000 " && \
        echo "---" && \
        echo "Or connect to the Underpass database:" && \
        echo "postgresql://underpass@localhost:5439/underpass" && \
        echo "---" && \
        echo "Stop services: docker/services-stop.sh" && \
        echo "Start services: docker/services-start.sh" && \
        echo "Terminal login: docker exec -w /code -ti underpass /bin/bash" && \
        echo ""
    fi
else
    echo "Usage: docker/install.sh -r <region> -c <country>"
    echo "Example: docker/install.sh -r asia -c nepal"
    echo ""
    echo "You'll need Docker and Docker Compose installed on your system."
fi

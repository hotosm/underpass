#!/bin/bash

REGION=${1:-asia}
COUNTRY=${2:-nepal}

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
docker exec -t underpass apt -y install osm2psql && \
echo "Downloading bootstrap sample data (Nepal) ..." && \
docker exec -t underpass cd utils && wget https://download.geofabrik.de/$REGION/$COUNTRY-latest.osm.pbf && \
docker exec -t underpass cd utils && wget https://download.geofabrik.de/$REGION/$COUNTRY.poly && \
docker exec -t underpass cd utils && wget python3 poly2geojson.py $COUNTRY.poly && \
docker exec -t underpass cd utils && cp $COUNTRY.geojson ../config/priority.geojson && \
docker exec -w /code/build -t underpass make install && \
docker exec -t underpass cd utils && osm2pgsql -H postgis -U underpass -p underpass -P 5432 -d underpass --extra-attributes --output=flex --style ./raw-underpass.lua nepal-latest.osm.pbf && \
docker exec -t underpass cd utils && psql -h postgis -p 5432 -w underpass underpass < raw-underpass.sql && \
docker exec -t underpass cd build ./underpass --bootstrap && \
echo "Starting replicator service ..." && \
docker exec -t underpass tmux new-session -d -s replicator 'cd /code/build && ./underpass -t $(date +%Y-%m-%dT%H:%M:%S -d "2 days ago")' && \
docker exec -t underpass tmux new-session -d -s rest-api 'cd /code/python/restapi && uvicorn main:app --reload --host 0.0.0.0' && \
docker exec -t underpass tmux new-session -d -s react-cosmos 'cd /code/js && yarn cosmos' && \
echo "Done! 🚀" && \
echo "Underpass OSM Planet replicator is now running on a Docker container" && \
echo "---" && \
echo "Check your browser: http://127.0.0.1:5000 " && \
echo "REST API: http://127.0.0.1:8000 " && \
echo "---" && \
echo "Or connect to the Underpass database:" && \
echo "postgresql://underpass@postgis/underpass" && \
echo ""
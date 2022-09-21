#!/bin/bash
echo "Composing containers ..." && \
docker-compose up -d && \
echo "Building Underpass ..." && \
docker exec -w /code -t underpass ./autogen.sh && \
docker exec -w /code -t underpass mkdir -p /code/build && \
docker exec -w /code/build -t underpass ../configure CXXFLAGS="-std=c++17 -g -O0" && \
docker exec -w /code/build -t underpass make -j2 && \
docker exec -w /code/build -t underpass make install && \
docker exec -t underpass mkdir -p /usr/local/lib/underpass/ && \
echo "Installing Postgres ..." && \
docker exec -t underpass apt update && \
docker exec -t underpass apt -y install postgresql postgresql-contrib && \
echo "Creating Galaxy database ..." && \
docker exec -t underpass psql -U underpass -c 'create database galaxy';
echo "Setting up Galaxy database ..." && \
docker exec -w /code/data -t underpass psql -U underpass galaxy -f galaxy.sql && \
echo "Setting up config ..." && \
docker exec -t underpass cp /code/docker/underpass-config.yaml /root/.underpass && \
echo "Done!"

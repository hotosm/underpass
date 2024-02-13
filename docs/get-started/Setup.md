## Boostrap the database

You can prepare your Underpass installation with data for a specific country.

### Pre-requisites

#### Database

Prepare your PostgreSQL + PostGIS database, for example:

```
sudo apt update
sudo apt install postgis
sudo su - postgres
psql
postgres=# CREATE USER underpass WITH PASSWORD 'your_password';
postgres=# CREATE DATABASE underpass;
postgres=# GRANT ALL PRIVILEGES ON DATABASE "underpass" to underpass;
postgres=# ALTER ROLE underpass SUPERUSER;
exit
psql postgresql://underpass:your_password@localhost:5432/underpass < setup/underpass.sql
```

#### Requirements

```
sudo apt install python3-pip -y
sudo apt install python3.11-venv
source ~/venv/bin/activate
pip install fiona
pip install shapely
apt install osm2pgsql
```

### Bootstrap

Now you can go to the `utils` directory and run the boostrap script:

cd utils
chmod +x bootstrap.sh
./bootstrap.sh -r south-america -c uruguay -p 5432 -u underpass
```

Use `-p <PORT>` and `-u <USERNAME>` for the database.

Regions (-r) are:

    africa
    asia
    australia-oceania
    central-america
    europe
    north-america
    south-america

Countries (-c) is the name of the country inside the region.

Data is downloaded from GeoFabrik, if you are not sure of what name you need to use, please check there.

For advanced users, check the [boostrap script documentation](/underpass/Dev/bootstrapsh).


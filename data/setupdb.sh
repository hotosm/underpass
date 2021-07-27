#!/bin/bash
#
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
#

#
# This assume the user running this script has full priviledges for the database.
#
usage()
{
    echo "$0 "

    cat <<EOF
--help(-h)             - Display usage
--user(-u)	       - Database user name
--passwd(-p)	       - Database password
--recreate(-r)         - Recreate database if it exists
This program is a simple utility to setup a database properly for Underpass.
EOF
    exit
}

dropdb="yes"
datahost="https://www.senecass.com/projects/Mapping/underpass/"

if test -e /etc/default/underpass; then
    . /etc/default/underpass
    dbhost="${PGHOST}"
    dbuser="${PGUSER}"
    dbpass="${PGPASSWORD}"
else 
    dbhost="localhost"
    dbuser="postgres"
    dbpass=""
fi

OPTS="`getopt -o hu:s:p:r,d: -l help,dbuser:,dbpasswd:,dbserver:,recreate,datahost:`"
while test $# -gt 0; do
    case $1 in
	-s|--dbserver) dbhost=$2;;
	-u|--dbuser) dbuser=$2;;
	-p|--dbpasswd) dbpass=$2 ;;
	-r|--recreate) dropdb="yes" ;;
	-d|--datahost) datahost=$2 ;;
        -h|--help) usage ;;
        --) break ;;
    esac
    shift
done

if test x"${dbhost}" = x"localhost"; then
    host=""
else
    host="--host=${dbhost}"
fi
if test x"${dbuser}" != x"postgres" -a x"${dbuser}" != x"root"; then
    user="--username=${dbuser}"
else
    user=""
fi

if test $(grep -c postgres /etc/passwd) -eq 0; then
    echo "You need to install postgresql-all to create the databases"
    echo "Once installed, execute /usr/share/underpass/setupdb.sh"
    exit 0;
fi

# Create the user account
sudo -u postgres createuser --createdb underpass
if test $? -gt 1; then
    echo "WARNING: createuser ${dbuser} failed!"
fi
sudo -u postgres psql -c "ALTER USER ${dbuser} WITH PASSWORD \'${dbpass}\'" >& /dev/null

# Note that the user running this script must have the right permissions.

databases="underpass pgsnapshot osmstats"

for dbname in ${databases}; do
    echo "Setting up database ${dbname}..."
    if test x"${dropdb}" = x"yes"; then
	sudo -u postgres dropdb ${host} --if-exists ${dbname} >& /dev/null
    fi
    exists="`psql ${host} ${user} -l | grep -c ${dbname}`"
    # exists=0
    if test "${exists}" -eq 0; then
	echo "Creating postgresql database ${dbname}"
	sudo -u postgres createdb ${host} -T template0 -O ${dbuser} ${dbname} >& /dev/null
	if test $? -gt 1; then
	    echo "WARNING: createdb ${dbname} failed!"
	    exit
	fi
	sudo -u postgres psql ${host} -d ${dbname} ${user} -c 'create extension hstore;' >& /dev/null
	if test $? -gt 2; then
	    echo "ERROR: couldn't add hstore extension!"
	    exit
	fi
	sudo -u ${user} psql ${host} -d ${dbname} ${user} -c 'create extension postgis;' >& /dev/null
	if test $? -gt 2; then
	    echo "ERROR: couldn't add postgis extension!"
	    exit
	fi
	sudo -u postgres psql ${host} -d ${dbname} ${user} -c 'create extension fuzzystrmatch;' >& /dev/null
	if test $? -gt 2; then
	    echo "ERROR: couldn't add fuzzystrmatch extension!"
	    exit
	fi
	# Populate the database with data
	sudo -u postgres psql ${host} -d ${dbname} -f /usr/share/underpass/${dbname}.sql
	if test $? -gt 2; then
	    echo "ERROR: couldn't add fuzzystrmatch extension!"
	    exit
	fi
    else
	echo "Postgresql database ${dbname} already exists, not creating."
    fi
done

if test x"$opts{'datahost']" = x; then
    echo -n "Do you want to download bootstrap data ? Files may be large: "
    read tmp
    if test x"${tmp}" = x -a x"${tmp}" != x"no"; then
	for dbname in ${databases}; do
	    echo "Downloading bootstrap data for database ${dbname}..."
	    url="https://$opts{'datahost']/underpass/${dbname}.sql.bz2"
	    wget -c --directory /tmp/ ${url}
	    if test $? > 0; then
		echo "ERROR: Couldn't download ${url}!"
	    fi
	done
	bunzip /tmp/${dbname}.sql
	sudo -u postgres psql ${host} -d ${dbname} ${user} -f /tmp/${dbname}.sql
    fi
fi

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

# This file is created at install time with all the database
# connection parameters
if test -e /etc/default/underpass; then
    . /etc/default/underpass
fi

OPTS="`getopt -o hu:s:p:r,d: -l help,recreate`"
while test $# -gt 0; do
    case $1 in
	-r|--recreate) dropdb="yes" ;;
        -h|--help) usage ;;
        --) break ;;
    esac
    shift
done

if test $(grep -c postgres /etc/passwd) -eq 0; then
    echo "You need to install postgresql-all to create the databases"
    echo "Once installed, execute /usr/share/underpass/setupdb.sh"
    exit 0;
fi

# Note that the user running this script must have the right permissions.

echo -n "Replace current authentication defaults (default yes)? "
read tmp
if test x"${tmp}" != x -a x"${tmp}" = x"no"; then
    exit
fi

# The prefix for the database connection parameter environment variables
dbs="UNDERPASS"

databases="underpass"
extensions="hstore postgis fuzzymatch"

for config in ${dbs}; do
    dbname=$(eval "echo \$$(echo ${config}_DBNAME)")
    dbhost=$(eval "echo \$$(echo ${config}_DBHOST)")
    dbuser=$(eval "echo \$$(echo ${config}_DBUSER)")
    dbpass=$(eval "echo \$$(echo ${config}_DBPASS)")
    if test x"${dbhost}" != x"localhost" -a x"${dbhost}" != x; then
	host="--host=${dbhost}"
    else
	host=""
    fi
    if test x"${dbuser}" != x; then
	user="--username=${dbuser}"
    fi

    echo "Setting up database ${dbname}..."
    sudo -u postgres dropdb ${host} --if-exists ${dbname} >& /dev/null
    exists="`psql ${host} ${user} -l | grep -c ${dbname}`"
    # exists=0
    if test "${exists}" -eq 0; then
	echo "Creating postgresql database ${dbname}"
	sudo -u postgres createdb ${host} -T template0 -O ${dbuser} ${dbname} >& /dev/null
	if test $? -gt 1; then
	    echo "WARNING: createdb ${dbname} failed!"
	    exit
	fi
	for ext in ${extensions}; do
	    sudo -u postgres psql ${host} -d ${dbname} ${user} -c "CREATE EXTENSION ${ext};" >& /dev/null
	    if test $? -gt 2; then
		echo "ERROR: couldn't add ${ext} extension!"
		exit
	    fi
	done
    else
	echo "Postgresql database ${dbname} already exists, not creating."
    fi
done

echo sudo -u postgres psql ${host} -d ${UNDERPASS_DBNAME}  --username ${UNDERPASS_DBUSER} -f /usr/share/underpass/underpass.sql >& /dev/null
if test $? -gt 2; then
    echo "ERROR: couldn't create database schema for Underpass!"
    exit
fi


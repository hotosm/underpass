#!/bin/bash
#
# Copyright (c) 2020, Humanitarian OpenStreetMap Team
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:

# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.

# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.

# * Neither the name of copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

declare -Ag opts
opts['dbhost']="localhost"
opts['dbuser']=""
opts['dbpass']=""
opts['dropdb']="no"

# OPTS="`getopt -o hu:i:p:r -l help,import,dbuser:,dbpasswd:,recreate`"
# while test $# -gt 0; do
#     case $1 in
# 	-u|--dbuser) dbuser=$2;;
# 	-b|--passwd) dbpass=$2 ;;
# 	-r|--recreate) dropdb="yes" ;;
# 	-i|--import) import="yes" ;;
#         -h|--help) usage ;;
#         --) break ;;
#     esac
#     shift
# done

# Note that the user running this script must have the right permissions.

databases="underpass pgsnapshot osmstats"

for dbname in ${databases}; do
    echo "Processing ${infile} into ${dbname}..."
    exists="`psql -l | grep -c ${dbname}`"
    if test "${exists}" -eq 0; then
	echo "Creating postgresql database ${dbname}"
	if test x"${dropdb}" = x"yes"; then
	    dropdb --if-exists ${dbname} >& /dev/null
	fi
	createdb -EUTF8 ${dbname} ${dbname} -T template0  >& /dev/null
	if test $? -gt 0; then
	    echo "WARNING: createdb ${dbname} failed!"
	    exit
	fi
	psql -d ${dbname} -c 'create extension hstore;' >& /dev/null
	if test $? -gt 0; then
	    echo "ERROR: couldn't add hstore extension!"
	    exit
	fi
	psql -d ${dbname} -c 'create extension postgis;' >& /dev/null
	if test $? -gt 0; then	
	    echo "ERROR: couldn't add postgis extension!"
	    exit
	fi
	psql -d ${dbname} -c 'create extension fuzzystrmatch;' >& /dev/null
	if test $? -gt 0; then	
	    echo "ERROR: couldn't add fuzzystrmatch extension!"
	    exit
	fi
	# Create the database schemas
	psql -d ${dbname} -f ${dbname}.sql
    else
	echo "Postgresql database ${dbname} already exists, not creating."
    fi
done

# osm2pgsql -x -c --slim -C 500 -d ${dbname} --number-processes 8 ${infile} --hstore --input-reader xml --drop >& /dev/null

# ogr2ogr -skipfailures -progress -overwrite -f  "PostgreSQL" PG:"dbname=${dbname}" -nlt GEOMETRYCOLLECTION ${infile} -lco COLUMN_TYPES="other_tags=hstore"


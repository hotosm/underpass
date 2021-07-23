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

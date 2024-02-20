#!/bin/bash

# Read destdir_base from Underpass YAML configuration file
destdir_base=$(grep -A 1 destdir_base /etc/underpass/default.yaml | tail -n 1 | sed 's/^[[:space:]]*- //')

# Get latest downloaded Changeset and OsmChange
changeseturl=$(find ${destdir_base:+$destdir_base}replication/changesets/ -type f -printf '%T@ %p\n' | sort -n | tail -n 1 | cut -d ' ' -f 2- | grep -oE '[0-9]{3}\/[0-9]{3}/[0-9]{3}')
minuteurl=$(find ${destdir_base:+$destdir_base}replication/minute/ -type f -printf '%T@ %p\n' | sort -n | tail -n 1 | cut -d ' ' -f 2- | grep -oE '[0-9]{3}\/[0-9]{3}/[0-9]{3}')

underpass -u "$minuteurl" --changeseturl "$changeseturl"
#!/usr/bin/python3
#
# Copyright (c) 2020, Humanitarian OpenStreetMap Team
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
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

import logging
import getopt
import osmium
import psycopg2
from sys import argv

from shapely import wkt, wkb
import shapely.geometry
from shapely.geometry import Point
import shapely.wkb as wkblib



class OsmFile(object):
    """OSM File output"""
    def __init__(self, filespec=None):
        if filespec:
            self.file = open(filespec, 'w')
            self.file.write('<?xml version=\'1.0\' encoding=\'UTF-8\'?>\n')
            self.file.write('<osm version="0.6" generator="export.py 0.1">\n')
    def footer(self):
        #logging.debug("FIXME: %r" % self.file)
        self.file.write("</osm>\n")
        if self.file != False:
            self.file.close()

    def writeNode(self, node):
        self.file.write("<node id='%s' lat='%f' lon='%f' action='modify' version='2' >" % (node['id'], node['geom'].y, node['geom'].x))
        for key,value in node.items():
            if key == 'id' or key == 'geom':
                continue
            else:
                self.file.write("\n  <tag k='%s' v='%s'/>" % (key, value))
        self.file.write("\n</node>\n")

    def writeWay(self, way):
        self.file.write("<way id='%s' action='modify' version='2' >" % (way['id']))
        for node in way['nodes']:
            self.file.write("\n  <nd ref='%s'/>" % node)
        # for key,value in way.items():
        #     if key == 'id':
        #         continue
        #     else:
        #         self.file.write("\n  <tag k='%s' v='%s'/>" % (key, value))
        self.file.write("\n</way>\n")

def usage():
    out = """
    help(-h)       Get command line options
    verbose(-v)    Enable verbose output
    database(-d)   Database to use
    nodefilter(-n) Filter for data queries
    wayfilter(-w)  Filter for data queries
    """
    print(out)

if len(argv) <= 1:
    usage()

options = dict()
options["database"] = "pgsnapshot";
options["nodefilter"] = "hospital";
options["wayfilter"] = "highway";
try:
    (opts, val) = getopt.getopt(argv[1:], "h,v,d,n,w",
        ["help", "verbose", "database", "nodefilter", "wayfilter"])
except getopt.GetoptError as e:
    logging.error('%r' % e)
    usage(argv)
    quit()

for (opt, val) in opts:
    if opt == '--help' or opt == '-h':
        usage(
        )
    elif opt == "--database" or opt == '-d':
        options['database'] = val.split(',')
    
connect = "dbname='" +  options['database'] + "'"
dbshell = psycopg2.connect(connect)
dbshell.autocommit = True
dbcursor = dbshell.cursor()

osm = OsmFile("example.osm")

# Get all the highways
# query = """SELECT tags->'name',nodes,tags,ST_AsEWKT(linestring) FROM ways WHERE tags->'highway' is not NULL AND tags->'highway'!='path' LIMIT 5;"""
query = """SELECT id,tags->'name',nodes,tags FROM ways WHERE tags->'highway' is not NULL AND tags->'highway'!='path' LIMIT 15;"""
dbcursor.execute(query)
all = dbcursor.fetchall()
for line in all:
    result = dict()
    result['id'] = line[0]
    result['name'] = line[1]
    result['nodes'] = line[2]
    result['tags'] = line[3]
    print("WAY: %s" % result['tags'])

    # Get the data for each node in the array
    if type(result['nodes']) != int:
        for data in result['nodes']:
            print(data)
            #            if data:
            query = """SELECT id,version,user_id,tstamp,changeset_id,tags,geom FROM nodes WHERE id=%d""" % data
            print(query)
            dbcursor.execute(query)
            entry = dbcursor.fetchone()
            node = dict()
            node['id'] = entry[0]
            node['geom'] =  wkb.loads(entry[6], hex=True)
            # print("NODE: %s" % line[6])
            osm.writeNode(node)
    line = dbcursor.fetchone()
    osm.writeWay(result)

osm.footer()

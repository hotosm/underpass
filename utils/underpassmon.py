#!/usr/bin/python3
#
# Copyright (c) 2020, 2021, 2023 Humanitarian OpenStreetMap Team
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

'''
    Replication process log monitor

    Use this script for monitoring Underpass logs and
    estimate replication speed.

    Monitoring OsmChanges processing (default):
    python underpassmon.py -f underpass.log

    Monitoring ChangeSets processing:
    python underpassmon.py -f underpass.log -m changesets
'''

import re
from datetime import datetime
import time
import argparse


SEARCH_STRING = {
    "osmchanges": "final_entry: ",
    "changesets": "last_closed_at: "
}

class UnderpassLogMonitor:

    def __init__(self, mode):
        self.start = (datetime.now(), None)
        self.lastDateTime = None
        self.elapsedTime = None
        self.processed = None
        self.searchString = SEARCH_STRING[mode]

    def getDateFromStr(self, str):
        return datetime.strptime(str, '%Y-%b-%d %H:%M:%S') 

    def getDateFromLogLine(self, logLine):
        result = re.search(self.searchString, logLine)
        if result:
            dateString = logLine[
                result.start() + len(self.searchString):result.start() +
                len(self.searchString) + 20
            ]
            return self.getDateFromStr(dateString)
        return None

    def getInfo(self, line):
        try:
            current = (datetime.now(), self.getDateFromLogLine(line))
            if current[1]:
                if self.start[1] == None:
                    self.start = current
                if self.lastDateTime == None or current[1] > self.lastDateTime:
                    self.lastDateTime = current[1]
                    self.processed = self.lastDateTime - self.start[1]
                    self.elapsedTime = datetime.now() - self.start[0]
        except:
            pass

    def follow(self, filename):
        with open(filename) as f:
                f.seek(0,2)
                while True:
                    pos = f.tell()
                    line = f.readline()
                    if not line:
                        f.seek(pos)
                        time.sleep(.2)
                    else:
                        self.getInfo(line)
                        self.dump()

    def dump(self):
        if self.processed:
            oneYear = (365*24*60*60 * self.elapsedTime.total_seconds() /  self.processed.total_seconds()) / 60 / 60
            print("Started:", self.start[1].strftime('%Y-%b-%d %H:%M:%S'))
            print("Current:", self.lastDateTime.strftime('%Y-%b-%d %H:%M:%S'))
            print("Processed [", self.processed, "hs ] in [", self.elapsedTime, "hs ]")
            print( "[EST] 1 year in ", round(oneYear,2), "hs")
            print("")

def main():
    args = argparse.ArgumentParser()
    args.add_argument("--file", "-f", help="Underpass logfile", type=str, default="underpass.log")
    args.add_argument("--mode", "-m", help="String for search before date", type=str, default="osmchanges")
    args = args.parse_args()
    ulog = UnderpassLogMonitor(args.mode)
    ulog.follow(args.file)

if __name__ == "__main__":
    main()

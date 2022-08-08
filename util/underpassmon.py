#!/usr/bin/python3
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
#     along with Underpass.  If not, see <https://www.gnu.org/licenses/>.

'''
    Replication process log monitor

    Use this script for monitoring Underpass logs and
    estimate replication speed.

    Usage example:

    1. Save timestamps to a temporary log file
    tail -f underpass.log | grep --line-buffered "final_entry" > ~/timing.log

    2. On a different terminal, run the log monitor
    python underpassmon.py --file ~/timing.log

    3. Once finished, remove temporary log file
    rm ~/timing.log
'''

import re
from datetime import datetime
import time
import argparse


class UnderpassLogMonitor:

    def __init__(self):
        self.start = (datetime.now(), None)
        self.lastDateTime = None
        self.elapsedTime = None
        self.processed = None

    def getDateFromStr(self, str):
        return datetime.strptime(str, '%Y-%b-%d %H:%M:%S') 

    def getDateFromLogLine(self, logLine):
        result = re.search("final_entry: ", logLine)
        if result:
            dateString = logLine[result.start() + 13:result.start() + 33]
            return self.getDateFromStr(dateString)
        return None

    def getInfo(self, line):
        current = (datetime.now(), self.getDateFromLogLine(line))
        if current[1]:
            if self. start[1] == None:
                self.start = current
            if self.lastDateTime == None or current[1] > self.lastDateTime :
                self.lastDateTime = current[1]
                self.processed = self.lastDateTime - self.start[1]
        self.elapsedTime = datetime.now() - self.start[0]

    def follow(self, filename):
        with open(filename) as f:
                f.seek(0,2)
                while True:
                    pos = f.tell()
                    line = f.readline()
                    if not line:
                        f.seek(pos)
                    else:
                        self.getInfo(line)
                        self.dump()
                    time.sleep(.2)

    def dump(self):
        if self.processed:
            oneYear = (365*24*60*60 * self.elapsedTime.total_seconds() /  self.processed.total_seconds()) / 60 / 60
            print("Processed [", self.processed, "hs ] in [", self.elapsedTime, "hs ]")
            print( "[EST] 1 year in ", round(oneYear,2), "hs")

def main():
    args = argparse.ArgumentParser()
    args.add_argument(
        "--file", help="Underpass logfile", type=str, default="underpass.log"
    )
    args = args.parse_args()
    ulog = UnderpassLogMonitor()
    ulog.follow(args.file)

if __name__ == "__main__":
    main()

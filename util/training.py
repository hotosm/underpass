#!/usr/bin/python3

# Copyright (c) 2021 Humanitarian OpenStreetMap Team
#
# This file is part of Odkconvert.
#
#     stats2galaxy is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     Odkconvert is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with Odkconvert.  If not, see <https:#www.gnu.org/licenses/>.
#

import argparse
import psycopg2
import sys
import os
import epdb
import logging
import pandas as pd
import pathlib
import csv


class Training(object):
    def __init__(self, db=None, host=None):
        """ """

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='import training spreadsheet into Galaxy database')
    parser.add_argument("--infile","-i", help='The input spreadsheet')
    parser.add_argument("--verbose","-v", default='yes', help='Enable verbosity')
    args = parser.parse_args()

    # if verbose, dump to the terminal as well as the logfile.
    if args.verbose == "yes":
        if os.path.exists('training.log'):
            os.remove('training.log')
        logging.basicConfig(filename='training.log', level=logging.DEBUG)
        root = logging.getLogger()
        root.setLevel(logging.DEBUG)

        ch = logging.StreamHandler(sys.stdout)
        ch.setLevel(logging.DEBUG)
        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
        ch.setFormatter(formatter)
        root.addHandler(ch)


    if os.path.splitext(args.infile)[1] == ".xlsx":
        df = pd.read_excel(args.infile)
        df_sheet_all = pd.read_excel(args.infile, sheet_name=None)
        for name,foo in df_sheet_all.items():
            #print(name)
            sheet = pd.read_excel(args.infile, sheet_name='20210423')
            for col in df.columns:
                print(col)
            df.dropna(how="any")
            head1 = df.iloc[0]
            df = df[1:]
            head2 = df.iloc[6]
            #print(head2)
            df.columns = head2
            #print(df)
            #print(sheet)
            break
    else:
        df = pd.read_csv(args.infile)
        df.dropna(how="any")
        head1 = df.iloc[0]
        print(head1)
        # df = df[1:]
        # head2 = df.iloc[7]
        # print("========================")
        # print(head2)
        # df.columns = head2
        #print(df)
        for col in df.columns:
            print(col)

        for index, row in df.iterrows():
            data = dict()
            #print(index)
            #print('~~~~~~')
            #print(row)
            if type(row[0]) == str:
                if row[0].isdigit():
                    i = 0
                    while i < head2.size:
                        print("FIXME: %r" % row[i])
                        if type(head2[i]) == float:
                            head2[i] = "id"
                        data[head2[i]] = row[i]
                        i = i + 1
                    print(data)
                else:
                    head2 = df.iloc[index + 1]
                    df.columns = head2
            print('------')        # # Drop rows which contain any NaN values

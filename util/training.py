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
import sys
import os
import epdb
import logging
import enum
import pandas as pd
from sqlalchemy import create_engine, text
from sqlalchemy import Table, Column, MetaData, Integer, String, ARRAY, Boolean, DateTime
from sqlalchemy import Enum, BigInteger
from sqlalchemy.dialects.postgresql import insert


#
# These enums match what the database schema also supports
#
class GenderEnum(enum.Enum):
    male = 'male'
    female = 'female'
    nonbinary =  'nonbinary'
    other = 'other'
    private = 'private'

class AgeEnum(enum.Enum):
    child = '10-14'
    teen = '15-24'
    adult = '25-40'
    mature = '41+'

    
class DesignationEnum(enum.Enum):
     volunteer = 'volunteer'
     intern = 'intern'
     civil = 'civil'
     gis = 'gis'
     director = 'director'

class Training(object):
    def __init__(self, db=None, host=None):
        """Parse training data"""
        self.data = dict()
        conn = "postgresql+psycopg2://"
        if host is not None:
            conn += host + "/"
        else:
            conn += "localhost/"
        if db is None:
            conn += "galaxy"
        else:
            conn += db
        self.engine = create_engine(conn)
        usermeta = MetaData()
        self.users = Table(
            "users",
            usermeta,
            Column('id', BigInteger),
            Column('name', String),
            Column('gender', Enum(GenderEnum)),
            Column('agerange', Enum(AgeEnum)),
            Column('topics', ARRAY(String)),
            Column('country', String),
            Column('designation', Enum(DesignationEnum)),
            Column('organization', Integer),
            Column('youthmapper', Boolean),
            )
        trainmeta = MetaData()
        self.training = Table(
            "training",
            trainmeta,
            Column('name', String),
            Column('topics', ARRAY(String)),
            Column('location', String),
            Column('type', Boolean),
            Column('organization', Integer),
            Column('hours', Integer),
            Column('timestamp', DateTime),
            )
        with self.engine.connect() as conn:
            sql = insert(self.users).values(name='foobar', id=123456)
            sql = sql.on_conflict_do_update(
                index_elements=['id'],
                set_=dict(id=123456, name='barfoo', gender='nonbinary')
            )
            print(sql)
            result = conn.execute(sql)
        #     print(result.all())

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='import training spreadsheet into Galaxy database')
    parser.add_argument("--infile","-i", help='The input spreadsheet')
    parser.add_argument("--dburl","-d", help='The output database URL')
    parser.add_argument("--verbose","-v", default='yes', help='Enable verbosity')
    args = parser.parse_args()

    training = Training()

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
    else:
        df = pd.read_csv(args.infile)
        
    df.dropna(how="any")
    head1 = df.iloc[0]
    print(head1)
    for col in df.columns:
        print(col)
        
    for index, row in df.iterrows():
        data = dict()
        if type(row[0]) == str:
            if row[0].isdigit():
                i = 0
                while i < head2.size:
                    # print("FIXME: %r" % row[i])
                    if type(head2[i]) == float:
                        head2[i] = "id"
                    data[head2[i]] = row[i]
                    i = i + 1
                print(data)
            else:
                head2 = df.iloc[index + 1]
                df.columns = head2

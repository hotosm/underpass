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
import re
import pandas as pd
from sqlalchemy import create_engine, text
from sqlalchemy import Table, Column, MetaData, Integer, String, ARRAY, Boolean, Date
from sqlalchemy import Enum, BigInteger, Sequence
from sqlalchemy.dialects.postgresql import insert
from sqlalchemy.exc import SQLAlchemyError


#
# These enums match what the database schema also supports
#
class UnitEnum(enum.Enum):
    country = 'country'
    region = 'region'
    microgrant = 'microgrant'
    organization = 'organization'
    osm = 'osm'
    boundary = 'boundary'
    campaign = 'campaign'
    hot = 'hot'

class SegmentEnum(enum.Enum):
    new_existing = 'new_existing'
    youth = 'youuth_mappers'
    ngo = 'ngo'
    government = 'government'

class GenderEnum(enum.Enum):
    male = 'male'
    female = 'female'
    nonbinary =  'nonbinary'
    other = 'other'
    others = 'others'
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
     student = 'student'
     trainer = 'trainer'
     researcher = 'researcher'

class TypeEnum(enum.Enum):
    virtual = 'virtual'
    inperson = 'inperson'

class TopicEnum(enum.Enum):
    remote = 'remote'
    field = 'field'
    other = 'other'


class Training(object):
    def __init__(self, dburl=None):
        """Initialize training database"""
        self.data = dict()
        conn = "postgresql+psycopg2://" + dburl
        try:
            # self.engine = create_engine(conn, echo=True)
            self.engine = create_engine(conn)
            logging.info("Connected to: %r" % conn)
        except InternalError:
            logging.error("Couldn't connect to " + conn)
        orgmeta = MetaData()
        self.orgs = Table(
            "organizations",
            orgmeta,
            # Column('oid', Integer, Sequence('oid_seq')),
            Column('oid', Integer),
            Column('name', String),
            Column('unit', Enum(UnitEnum)),
            Column('trainee', Enum(SegmentEnum)),
            )
        usermeta = MetaData()
        self.users = Table(
            "users",
            usermeta,
            Column('id', BigInteger),
            Column('name', String),
            Column('username', String),
            Column('gender', Enum(GenderEnum)),
            Column('agerange', Enum(AgeEnum)),
            Column('topics', ARRAY(String)),
            Column('country', String),
            # Column('designation', Enum(DesignationEnum)),
            Column('designation', String),
            Column('organization', Integer),
            Column('trainings', Integer),
            Column('marginalized', String),
            Column('youthmapper', Boolean),
            )
        trainmeta = MetaData()
        self.training = Table(
            "training",
            trainmeta,
            # Column('tid', Integer, Sequence('tid_seq')),
            Column('tid', Integer),
            Column('name', String),
            Column('topics', ARRAY(String)),
            Column('location', String),
            Column('organization', Integer),
            Column('eventtype', Enum(TypeEnum)),
            Column('topictype', Enum(TopicEnum)),
            Column('hours', Integer),
            Column('date', String),
            )

    def getCountryID(self, name=None):
        with self.engine.connect() as conn:
            if name and type(name) == str:
                sql = "SELECT cid FROM geoboundaries WHERE name=\'" + name + "\';"
                result = conn.execute(text(sql))
                ans = result.fetchone()
                if ans is None:
                    return 0
            return ans[0]

    def getUserID(self, name=None):
        with self.engine.connect() as conn:
            if name and type(name) == str:
                sql = "SELECT id FROM users WHERE name=\'" + name + "\';"
                result = conn.execute(text(sql))
                ans = result.fetchone()
                if ans is None:
                    sql = "SELECT COUNT(id)+1 FROM users;"
                    result = conn.execute(text(sql))
                    ans = result.fetchone()
                if ans[0] is None:
                    return 0
            return ans[0]

    def getOrgID(self, name=None):
        with self.engine.connect() as conn:
            if type(name) == float:
                return None
            sql = "SELECT oid FROM organizations WHERE name=\'" + name + "\';"
            result = conn.execute(text(sql))
            ans = result.fetchone()
            if ans is None:
                sql = "SELECT COUNT(oid)+1 FROM organizations;"
                result = conn.execute(text(sql))
                ans = result.fetchone()
                if ans[0] is None:
                    return 0
            return ans[0]

    def getTrainingID(self, data=None):
        with self.engine.connect() as conn:
            if 'date' not in data or type(data['name']) == float:
                return 0
            try:
                sql = "SELECT tid FROM training WHERE name=\'" + data['name'] + "\' AND date=\'" + str(data['date']) + "\';"
            except e:
                print("ERROR: %r" % e)
                return 0

            try:
                result = conn.execute(text(sql))
            except SQLAlchemyError as e:
                print("ERROR: %r" % e)
                return 0
            ans = result.fetchone()
            if ans is None:
                sql = "SELECT MAX(tid)+1 FROM training;"
                result = conn.execute(text(sql))
                ans = result.fetchone()
                if ans[0]is None:
                    return 0
            return ans[0]

    def orgColumns(self, org=dict()):
        for col in train:
            print(col)

    def trainingColumns(self, train=dict()):
        """Convert the strings from the spreadsheet header into the database column"""
        newtrain = dict()
        for col in train:
            if type(col) == float:
                continue
            if type(train[col]) == float:
                continue
            if type(col) == int:
                continue
            if type(col) == float:
                newtrain['topics'] = train[col]
                continue
            reg = re.compile("^Event number")
            if reg.match(col):
                continue
            reg = re.compile("^Name .*")
            if reg.match(col):
                newtrain['name'] = train[col]
                continue
            reg = re.compile("^Date .*")
            if reg.match(col):
                newtrain['date'] = train[col]
                continue
            reg = re.compile("^.* location")
            if reg.match(col):
                newtrain['location'] = train[col]
                continue
            reg = re.compile("^.* type")
            if reg.match(col):
                if train[col].lower() == "face-to-face":
                    newtrain['eventtype'] = "inperson"
                    continue
                if train[col].lower() == "remote":
                    newtrain['eventtype'] = "virtual"
                    continue
                continue
            reg = re.compile("^Topic")
            if reg.match(col):
                if train[col].lower() == "remote mapping":
                    newtrain['topictype'] = "remote"
                    continue
                if train[col].lower() == "field mapping":
                    newtrain['topictype'] = "field"
                    continue
                if train[col].lower() == "other":
                    newtrain['topictype'] = "other"
                    continue
        logging.debug(newtrain)
        return newtrain

    def userColumns(self, user=dict()):
        """Convert the strings from the spreadsheet header into the database column"""
        newuser = dict()
        i = 0
        for col in user:
            if str(col) == 'nan':
                continue
            if col == 'id':
                newuser['id'] = user[col]
                continue
            reg = re.compile("^Name .*")
            if reg.match(col):
                newuser['name'] = user[col]
                continue
            reg = re.compile("^OSM .*")
            if reg.match(col):
                newuser['username'] = user[col]
                continue
            reg = re.compile("^Designation .*")
            if type(user[col]) == float:
                newuser['designation'] = ""
            else:
                newuser['designation'] = user[col]
            # FIXME: waiting for input about defined values...
            # if reg.match(col):
            #     reg = re.compile("^Geomatics .*")
            #     if re.match(user[col]):
            #         newuser['designation'] = "gis"
            #         continue
            #     reg = re.compile("^Civil .*")
            #     if re.match(user[col]):
            #         newuser['designation'] = "civil"
            #         continue
            #     reg = re.compile(".* Director")
            #     if re.match(user[col]):
            #         newuser['designation'] = "director"
            #         continue
            #     reg = re.compile("^Research .*")
            #     if re.match(user[col]):
            #         newuser['designation'] = "director"
            #         continue
            reg = re.compile("^Country .*")
            if reg.match(col):
                cid = training.getCountryID(user[col])
                if cid is None:
                    newuser['country'] = 0
                else:
                    newuser['country'] = cid
                # newuser['country'] = user[col]
                continue
            reg = re.compile("^Are you a Youth .*")
            if reg.match(col):
                if user[col] == "Yes":
                    newuser['youthmapper'] = True
                elif user[col] == "No":
                    newuser['youthmapper'] = False
                continue

            reg = re.compile("^Is this .*")
            if reg.match(col):
                if user[col] == "Yes":
                    newuser['trainings'] = 1
                elif user[col] == "No":
                    newuser['trainings'] = 0
                continue
            # reg = re.compile("^Are you a member .*")
            reg = re.compile("If YES, .*")
            if reg.match(col):
                if type(user[col]) != float:
                    newuser['marginalized'] = user[col]
                else:
                    newuser['marginalized'] = ""
                continue
            reg = re.compile("^Gender")
            if reg.match(col):
                reg = re.compile("^Prefer .*")
                if reg.match(user[col]):
                    user[col] = "private"
                elif user[col].lower() == "non-binary":
                    user[col] = "nonbinary"
                elif user[col].lower() == "others":
                    user[col] = "other"
                newuser['gender'] = user[col].lower()
                continue
            reg = re.compile("^Age .*")
            if reg.match(col):
                if col == "10-14":
                    newuser['agerange'] = 'child'
                elif col == "15-24":
                    newuser['agerange'] = 'teen'
                elif col == "25-40":
                    newuser['agerange'] = 'adult'
                elif col == "40+":
                    newuser['agerange'] = 'mature'
                continue
            #newuser[col] = user[col]
            # FIXME: there may be multiple organizations, delimited by a comma
            reg = re.compile("^Organ.*")
            if reg.match(col):
                oid = self.getOrgID(user[col])
                neworg = dict()
                if oid is not None:
                    neworg['oid'] = oid
                    neworg['name'] = user[col].strip()
                    self.updateOrganization(neworg)
                continue
            i = i + 1

        return newuser

    def updateTraining(self, training=dict()):
        """Update the training table in the galaxy database"""
        if len(training) == 0:
            return None
        values = self.trainingColumns(training)
        values['tid'] = self.getTrainingID(values)
        # print(values)
        with self.engine.connect() as conn:
            sql = insert(self.training).values(values)
            sql = sql.on_conflict_do_update(
                index_elements=['tid'],
                set_=dict(values)
            )
            result = conn.execute(sql)

    def updateUser(self, user=dict()):
        """Update the user table in the galaxy database"""
        values = self.userColumns(user)
        if values['gender'] == "nan":
            return
        values['id'] = self.getUserID(values['name'])
        with self.engine.connect() as conn:
            sql = insert(self.users).values(values)
            sql = sql.on_conflict_do_update(
                index_elements=['id'],
                set_=dict(values)
            )
            result = conn.execute(sql)

    def updateOrganization(self, data=dict()):
        if data['name'] == "-":
            return None
        with self.engine.connect() as conn:
            sql = insert(self.orgs).values(data)
            sql = sql.on_conflict_do_update(
                index_elements=['name'],
                set_=dict(data)
            )
            result = conn.execute(sql)

    def parseFile(self, df=None):
        df.dropna(how="all")
        head1 = df.iloc[0]
        data = dict()
        head2 = df.iloc[6]
        for index, row in df.iterrows():
            logging.debug(row)
            if len(row) <= 1:
                break
            if 'Events log' not in row:
                continue
            if str(row['Events log']) == 'nan':
                # print(row)
                if str(row[1]) == 'nan':
                    self.updateTraining(data)
                    data.clear()
                    continue
                key = row[1]
                value = row[2]
                data[key] = value
                continue
            if type(row[0]) == int or type(row[0]) == float:
                i = 0
                while i < head2.size:
                    if type(head2[i]) == float:
                        head2[i] = "id"
                    data[head2[i]] = str(row[i])
                    i = i + 1
                self.updateUser(data)
            else:
                head2 = df.iloc[index + 1]
                df.columns = head2
                logging.debug("Header fields:\n%r" % head2)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='import training spreadsheet into Galaxy database')
    parser.add_argument("--infile","-i", help='The input spreadsheet')
    parser.add_argument("--dburl","-d", help='The output database URL')
    parser.add_argument("--verbose","-v", default='no', help='Enable verbosity')
    args = parser.parse_args()

    training = Training(args.dburl)

    # if verbose, dump to the terminal as well as the logfile.
    if os.path.exists('training.log'):
        os.remove('training.log')
    logging.basicConfig(filename='training.log', level=logging.DEBUG)
    root = logging.getLogger()
    #root.setLevel(logging.DEBUG)

    if args.verbose != "no":
        ch = logging.StreamHandler(sys.stdout)
        ch.setLevel(logging.DEBUG)
        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
        ch.setFormatter(formatter)
        root.addHandler(ch)


    if os.path.splitext(args.infile)[1] == ".xlsx":
        df = pd.read_excel(args.infile)
        df_sheet_all = pd.read_excel(args.infile, sheet_name=None)
        for name,foo in df_sheet_all.items():
            if name == "Summary" or name == "Instructions":
                continue
            logging.info("Processing sheet \'%s\'" % name)
            sheet = pd.read_excel(args.infile, sheet_name=name)
            training.parseFile(sheet)
    else:
        df = pd.read_csv(args.infile)
        logging.info("Processing file \'%s\'" % args.infile)
        training.parseFile(df)

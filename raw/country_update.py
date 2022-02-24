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


"""[Responsible for Country Update of Rawdata Table's , It will be one time update for all elements ]
Raises:
    err: [Database Connection Error]
    err: [Null Query Error]
Returns:
    [result]: [geom column Populated]
"""

import time
import logging
from psycopg2 import *
from psycopg2.extras import *
import sys
from enum import Enum
import datetime
import argparse
from configparser import ConfigParser

config = ConfigParser()
config.read("config.txt")

from dateutil.relativedelta import relativedelta
logging.basicConfig(format='%(asctime)s - %(message)s', level=logging.DEBUG)


class BatchFrequency(Enum):
    DAILY = 'd'
    WEEKLY = 'w'
    MONTHLY = 'm'
    QUARTERLY = 'q'
    YEARLY = 'y'
    
    def __str__(self):
        return self.value

def assign_end_wrt_frequency(start, frequency):
#     logging.debug( f"""frequency Osm  {frequency}""")
        
    if frequency == BatchFrequency.YEARLY:
        end = start-relativedelta(years=1)
    if frequency == BatchFrequency.MONTHLY:
        end = start-relativedelta(months=1)
    if frequency == BatchFrequency.QUARTERLY:
        end = start-relativedelta(months=4)
    if frequency == BatchFrequency.WEEKLY:
        end = start-relativedelta(days=7)
    if frequency == BatchFrequency.DAILY:
        end = start-relativedelta(days=1)
    return end


class Database:
    """[Database Class responsible for connection , query execution and time tracking, can be used from multiple funtion and class returns result ,connection and cursor]
    """

    def __init__(self, db_params):
        """Database class constructor"""

        self.db_params = db_params
        print('Database class object created...')

    def connect(self):
        """Database class instance method used to connect to database parameters with error printing"""

        try:
            self.conn = connect(**self.db_params)
            self.cur = self.conn.cursor(cursor_factory=DictCursor)
            logging.debug('Database connection has been Successful...')
            return self.conn, self.cur
        except OperationalError as err:
            """pass exception to function"""
            # set the connection to 'None' in case of error
            self.conn = None
            raise err

    def executequery(self, query):
        """ Function to execute query after connection """
        # Check if the connection was successful
        try:
            if self.conn != None:
                self.cursor = self.cur
                if query != None:
                    # catch exception for invalid SQL statement

                    try:
                        logging.debug('Query sent to Database')
                        self.cursor.execute(query)
                        self.conn.commit()
                        # print(query)
                        try:
                            result = self.cursor.fetchall()
                            logging.debug('Result fetched from Database')
                            return result
                        except:
                            return self.cursor.statusmessage
                    except Exception as err:
                        raise err

                else:
                    raise ValueError("Query is Null")
            else:
                print("Database is not connected")
        except Exception as err:
            print("Oops ! You forget to have connection first")
            raise err

    def close_conn(self):
        """function for clossing connection to avoid memory leaks"""

        # Check if the connection was successful
        try:
            if self.conn != None:
                if self.cursor:
                    self.cursor.close()
                    self.conn.close()
                    logging.debug("Connection closed")
        except Exception as err:
            raise err


class Underpass:
    """This class connects to Underpass database and responsible for Values derived from database"""

    def __init__(self, parameters=None):
        self.database = Database(dict(config.items("local")))
        self.con, self.cur = self.database.connect()
        self.params = parameters

    def getMax_timestamp(self,table):
        """Function to extract latest maximum osm element id and minimum osm element id present in  Table"""

        query = f"""select min("timestamp") as minimum , max("timestamp") as maximum from  {table};"""
        record = self.database.executequery(query)
        logging.debug(
            f"""Maximum osm_poly  timestamp fetched is {record[0][1]} and minimum is  {record[0][0]}""")
        return record[0][1], record[0][0]

    def update_country(self, start, end,table):
        """Function that updates Country column of table"""
        query = f"""update
	{table} as w
    set
        country = (
        select
            b.id
        from
            boundaries b
        where
            ST_Intersects( w.geom ,
            ST_SetSRID(b.boundary,
            4326))
        limit 1	)::int
    where country is null
    and "timestamp" >= '{start}'
    and "timestamp" < '{end}'"""
        result = self.database.executequery(query)
        logging.debug(f"""Changed Row : {result}""")

    def batch_update(self, start_batch_date, end_batch_date, batch_frequency,table_name):
        """Updates Country with  given start timestamp (python datetime format) , end timestamp along with batch frequency , Here Batch frequency means frequency that you want to run a batch with, Currently Supported : DAILY,WEEKLY,MONTHLY,QUARTERLY,YEARLY Only Supports with default Python Enum Type input (eg: BatchFrequency.DAILY). This function is made with the purpose for future usage as well if we want to update specific element between timestamp"""
        # BatchFrequency.DAILY
        
        if start_batch_date is None : 
            start_batch_date,end=self.getMax_timestamp(table_name)
        if end_batch_date is None : 
            start,end_batch_date=self.getMax_timestamp(table_name)
        batch_start_time = time.time()
        # Type checking
        if not isinstance(batch_frequency, BatchFrequency):
            raise TypeError('Batch Frequency Invalid')
        # Considering date is in yyyy-mm-dd H:M:S format
        logging.debug(
            f"""----------Update Country Function has been started for {start_batch_date} to {end_batch_date} with batch frequency {batch_frequency.value}----------""")
        looping_date = start_batch_date
        loop_count = 1
        while looping_date >= end_batch_date:
            start_time = time.time()
            start_date = looping_date
            end_date = assign_end_wrt_frequency(start_date, batch_frequency)
            self.update_country(end_date,start_date,table_name)
            logging.debug(
                f"""Batch {loop_count} Country Update from {end_date} to {start_date} , Completed in {(time.time() - start_time)} Seconds""")
            loop_count += 1
            looping_date = end_date

        # closing connection
        self.database.close_conn()
        logging.debug(
            f"""-----Updating Country Took-- {(time.time() - batch_start_time)} seconds for {start_batch_date} to {end_batch_date} with batch frequency {batch_frequency.value} -----""")


# The parser is only called if this script is called as a script/executable (via command line) but not when imported by another script
if __name__ == '__main__':
    #connection to the database
    connect = Underpass()
    """You can get min and max timestamp available in the table as well which will be default or you can pass it through arguments"""
    argParser = argparse.ArgumentParser(description="Updates Country column of Table")
    argParser.add_argument('-start', '--start', action='store',type=lambda s: datetime.datetime.strptime(s, '%Y-%m-%d'), dest='start',default=None, help='The start date of updating Country, Default is minimum timestamp of table')
    argParser.add_argument('-end', '--end', action='store',type=lambda s: datetime.datetime.strptime(s, '%Y-%m-%d'), dest='end',default=None, help='The end date of updating Country , Default is maximum timestamp of table')
    argParser.add_argument('-f', '--f', action='store', type=BatchFrequency, choices=list(BatchFrequency), dest='f',default='d', help='Frequency for Batch, Default is Monthly')
    argParser.add_argument('-table', '--table', action='store',  dest='table',default='nodes', help='Table Name to update')

    args = argParser.parse_args()
    try:
        # Note : You can not run function forward , if you want to update Country of 2020 you need to pass  2020-12-30 to 2020-01-01
        # """This function can be imported and reused in other scripts """
        connect.batch_update(args.start, args.end,
                            args.f,args.table)
    except Exception as e:
        logging.error(e)
        sys.exit(1)
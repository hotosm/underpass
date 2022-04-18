## Initialize Update script

Install psycopg2 and osmium in your python env

Create config.txt file in the directory and Put your credentials like this
```
[RAW]
host=localhost
user=
password=
dbname=
port=
```
Now run init , This will create 

```python rawdata-replication init ```

Now Run update with your style file -s parameter like this 

```python rawdata-replication update -s /home/user/underpass/raw/raw.lua```

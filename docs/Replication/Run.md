## Keep the database up-to-date minutely

Run this command for start processing data from 2 days ago

`underpass -t $(date +%Y-%m-%dT%H:%M:%S -d "2 days ago")`

On MacOS, the date command is different

`underpass -t $(date -v -2d +%Y-%m-%dT%H:%M:%S)`

Using Docker, add `docker exec -d -w /code/build underpass`

`docker exec -w /code/build underpass underpass -t $(date -v -2d +%Y-%m-%dT%H:%M:%S)`

For running underpass as a daemon, use the -d option:

`docker exec -d -w /code/build underpass underpass -t $(date -v -2d +%Y-%m-%dT%H:%M:%S)`


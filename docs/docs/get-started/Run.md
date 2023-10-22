## Keep the database up-to-date minutely

### Linux

Run this command for start processing data from 2 days ago:

`./underpass -t $(date +%Y-%m-%dT%H:%M:%S -d "2 days ago")`

### MacOS

On MacOS, the date command is slighty different:

`./underpass -t $(date -v -2d +%Y-%m-%dT%H:%M:%S)`

### Docker

If you're running Underpass on a Docker container:

`docker exec -w /code/build underpass ./underpass -t $(date +%Y-%m-%dT%H:%M:%S -d "2 days ago")`

For running as a daemon:

`docker exec -d -w /code/build underpass ./underpass -t $(date -v -2d +%Y-%m-%dT%H:%M:%S)`



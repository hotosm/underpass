# Tests for statistics collection

## Default

Run the default test, using testsuite/testdata/test_stats.osc for OsmChange file and 
testsuite/testdata/test_stats.yaml for validation.

./stats-test

## Stats validation from files

Extract stats from the first -f value and validate them against the second -f parameter.

./stats-test -f /stats/104497858.xml -f /stats/104497858.yaml

## Stats extraction from a single file

Extract stats from file and print the JSON result.

./stats-test -f /stats/104497858.xml

## Collect stats

Run a replicator process from timestamp, incrementing the path and collecting stats from 
OsmChange files. This is useful for doing a comparison with other sources (Insights). 
For more information about this feature, check this link.

./stats-test -m collect-stats -t 2021-05-11T17:00:00 -i 100

## Stats configuration file

Stats collection can be customized using a YAML configuration file. 

In the following example, we have a OsmChange file with several nodes created, using 
_building_ and _emergency_ keys for tags.

There are two different configurations for collecting stats from that file. 
On `statsconfig2.yaml` , _building_, _fire_station_, _hospital_ and _police_ 
are different categories of stats. 

`./testsuite/libunderpass.all/stats-test -f stats/test_statsconfig.osc \ 
    --statsconfigfile /testsuite/testdata/stats/statsconfig2.yaml`

Running the command above, you'll see this results:

```json
{
    "added":[
        {"building":1},
        {"fire_station":2},
        {"hospital":2},
        {"police":1}
    ], 
    "modified": []
}
```

You can assert this results adding another `-f` argument:

`./testsuite/libunderpass.all/stats-test -f stats/test_statsconfig.osc \
    --statsconfigfile /testsuite/testdata/stats/statsconfig2.yaml \
    -f stats/test_statsconfig2.yaml`

On the other configuration file (`statsconfig3.yaml`) there are two categories for stats: _building_ and _humanitarian_building_.

If you get stats from that file:

`./testsuite/libunderpass.all/stats-test -f stats/test_statsconfig.osc \
    --statsconfigfile /testsuite/testdata/stats/statsconfig2.yaml`

You'll see a different result:

```json
{
    "added":[
        {"building":2},
        {"humanitarian_building":4}
    ], 
    "modified": []
}
```

Again, you can assert the results:

`./testsuite/libunderpass.all/stats-test -f stats/test_statsconfig.osc \
    --statsconfigfile /testsuite/testdata/stats/statsconfig2.yaml \
    -f stats/test_statsconfig2.yaml`

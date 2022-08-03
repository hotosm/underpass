# Tests for statistics collection

As statistics are a key feature for Underpass, there are several ways to test
results and configurations.

## Default

The default test will use two files:

* OsmChange (testsuite/testdata/test_stats.osc)
* Stats results validation (testsuite/testdata/test_stats.yaml)

`./stats-test`

## More testing

You can run other existing tests or create your own custom ones.

### Stats extraction from a single file

Extract stats from file and print the JSON result:

`./testsuite/libunderpass.all/stats-test -f stats/107235440.xml`

```xml
{
    "added":[
        {"highway":8},
        {"highway_km":64917794}
    ],
    "modified":[
        {"highway":8}
    ]
}
```

## Stats validation from files

If you want to assert the results agains a YAML file, add second `-f` argument: 

`./testsuite/libunderpass.all/stats-test -f /stats/107235440.xml -f /stats/107235440.yaml`

```
    PASSED: Calculating added highway
    PASSED: Calculating modified highway
```

The YAML file contain the expected results:

```yaml
- modified_highway:
  - 8
- added_highway:
  - 8
```

### Collect stats

Run a replicator process from timestamp, incrementing the path and collecting stats from
OsmChange files. This is useful for doing a comparison with other sources (ex: Insights DB).

`./testsuite/libunderpass.all/stats-test -m collect-stats -t 2021-05-11T17:00:00 -i 10 > result.json`

### Stats configuration file

Stats collection can be customized using a YAML configuration file.

In the following example, we have an OsmChange file with several nodes created, using
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

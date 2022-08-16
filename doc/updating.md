## Updating configs and re-processing data

If you make changes to the stats configuration or the priority boundary, 
you'll may want to re-process all data.

### Priority boundary

Changes are filtered by `areaFilter()` for both Changesets and OsmChanges.

For adding a new country or expanding the priority boundary, just edit the [boundary file](https://github.com/hotosm/underpass/blob/master/data/priority.geojson) and run the replication process again.

### Stats configuration

Added and modified stats are produced from OsmChanges.

For changing the way Underpass collect statistics, just edit the [stats configuration file](https://github.com/hotosm/underpass/blob/master/validate/statistics.yaml) and run the replication process again,
only for OsmChanges

### Running the replication process

If you want to be precise, as _datetime to planet path_ calculation is not that exact right now,
you can pass both OsmChanges and Changesets paths to the replicator command.

In the following example, we want to re-process data exactly from `2014-01-01 00:00` to current time,
so we use the paths for OsmChanges (000/680/457) and ChangeSets (000/598/424) corresponding to that date:

```
./replicator -m -f minute -u 000/680/457 --changeseturl 000/598/424 --server $GALAXY_DB
```

For processing only OsmChanges, use the `--osmchanges` parameter.

```
./replicator -m -f minute -u 000/680/457 --osmchanges --server $GALAXY_DB
```

Remember that, if you want to disable `areaFilter()` you'll need to pass the correspondig 
`--osmnoboundary` (OsmChanges) or `--oscnoboundary` (ChangeSets) parameter:

```
./replicator -m --osmnoboundary -f minute -u 000/680/457 --osmchanges --server $GALAXY_DB
```

We currently use `--osmnoboundary` and run the [clean query](https://github.com/hotosm/underpass/blob/master/data/clean-osmchanges.sql) to have more accuracy, as `areaFilter()` will skip changes
with no coordinates (ex: when only the feature's tag changed).
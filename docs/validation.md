# Validating Data

It is important to make sure any data being added to OpenStreetMap is
of good quality. While there are number of other tools available to do
this, Underpass data validation is oriented towards real-time
analysis. Most all of the other tools have a time delay of hours or
days. As validation can be computationally intensive, there are two
levels of validation. Since Underpass is processing change files every
minute, the data validation must also fit within that time frame. Also
the data available in the minute updates is not often complete, so
some types of validation are impossible.

The first level of validation is simple, checking for the proper
values for tags. For buildings, the geometry can be checked to make
sure it's got 90 degree corners or is round. Also checks for
overlapping with other buildings in the same changeset is also
done.

The second level can't be done on huge datasets, but works well for
smaller datasets, for example a Tasking Manager project or a
county. This other level requires access to OSM data. Since querying a
large database may not be possible within the minute timeframe, this
is only enabled for datasets country sized or smaller. Using OSM data,
it's possible to identify duplicate buildings and highways.

The first level of data validation is applied to all changes, since it
can be completed within a minute. The second level of validation is
focused on Tasking Manager projects. The primary goal of this
secondary validation is to catch most mapping errors right away,

## The Database

All mapping errors are written to a table called *validation* in the
Galaxy database. This table contains the OSM ID of the feature, the
changeset ID that contains this change, and the user ID of the mapper
making the change. The status column is an array of the issues. Those
issues include bad building geometry, bad tag value, orphan node,
duplicate buildings, overlapping buildings, and incomplete
tagging. The location of the feature is a single node, which can be
used for spatial filtering.

In addition, there are currently two debugging columns in the
database. One is the timestamp of when the validation is performed,
and the other is the calculated angle for buildings that are
determined to have bad geometry, which is used to debug the calculation.

## Tag Validation

Organized mapping campaigns, especially those done on the ground using
(ODK)[https://opendatakit.org/] based mobile tools, have a defined set
of metadata tags. Often more detailed tags are added after remote
mapping by local mappers, as it's impossible to determine some
features from the satellite imagery, like building material.

HOT currently has a tool called
(MapCampaigner)[https://campaigns-staging.hotosm.org/], that is used
to validate tag completeness. Required tags are defined in a
(YAML)[https://www.yaml.org] config file. For example, to completely
map a building, the material of the walls of the building, the
material used for a roof, and whether it has walls. If the building is
an amenity, it should have a name and the type of amenity. The
existing list of tag values were based on what MapCampaigner uses, and
then extended by the Data Quality team at HOT. The current values are
listed in the config files:

[amenities](https://github.com/hotosm/underpass/blob/master/validate/amenity.yaml),
 [buildings](https://github.com/hotosm/underpass/blob/master/validate/building.yaml),
 [highways](https://github.com/hotosm/underpass/blob/master/validate/highway.yaml),
 [solid waste](https://github.com/hotosm/underpass/blob/master/validate/wastepoints.yaml),
 [landuse](https://github.com/hotosm/underpass/blob/master/validate/landuse.yaml)

By default, tag completeness is not enabled, as any data added by
remote mapping will always be missing tags, or have different
permissible values. Instead tag completeness can be used to monitor a
ground mapping campaign.

## Bad Geometry

Often buildings are added that don't have square corners. This can be
eliminated by using a plugin for some map editors like JOSM to force
all new buildings to have the correct geometry. There are other
plugins that allow the mapper or validator to correct the
geometry. Flagging this error while the mapper is still active allows
for educating the mapper about this issue so they can improve their
quality, which might be as simple as having them use a building
plugin.

To determine the building correctness, a corner is chosen, and the
angle between the two lines is calculated, A threshold is then applied
so any building that looks rectangular to the user is considered
acceptable. That threshold value is in the YAML file for building
validation so it's easy to adjust.

Since Underpass is using minute change files, sometimes it's difficult
to validate buildings that have been edited, as not all the nodes in
the building polygon are in the change file. Most remote mapping only
adds buildings, any editing is done later by a human validator.

## Highway Validation

Validating highways starts the correct tag value. More than validating
the values for the highway tag, this also will check the values of
important tags, like surface, smoothness, tracktype, and
sac_scale. The important validation is making sure any new highway
actually connects to the highway network. Otherwise navigation doesn't
work.

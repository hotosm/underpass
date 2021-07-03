# Validating Data

It is important to make sure any data beging added to OpenStreetMap is
of good quality.

In addition to validating the data, simple conflation is also
performed to look for duplicate buildings and highways. This is
potentially an expensive calculation

## tag completeness

Organized mapping campaigns, especially those done on the ground using
(ODK)[https://opendatakit.org/] bases mobile tools, have a defined set
of metadata tags. Often this is done after remote mapping by local
mappers, as it's impossible to determine some features from the
satellite imagery, like building material.

HOT currently has a tool called
(MapCampaigner)[https://campaigns-staging.hotosm.org/], that is used
to validate tag completeness. Required tags are defined in a
(YAML)[https://www.yaml.org] config file. For example, to completely
map a building, the material of the walls of the building, the
material used for a roof, and whether it has walls. If the building is
an amenity, it should have a name and the type of amenity.

## Bad Geometry

Often buildings are added that don't have square corners. This can be
eliminated by using a plugin for some map editors like JOSM to force
all new buildings to have the correct geometry. There are other
plugins that allow the mapper or validator to correct the
geometry. Flagging this error while the mapper is still active allows
for educating the mapper about this issue so they can improve their
quality, which might be as simple as having them use a building
plugin.

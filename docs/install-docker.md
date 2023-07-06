# Underpass Docker installation

This is the easiest way to get Underpass up and running.

Just select a region and run the installation script:

```sh
docker/install.sh -r asia -c nepal
```

Regions (-r) are:

* africa
* asia
* australia-oceania
* central-america
* europe
* north-america
* south-america

Countries (-c) is the name of the country inside the region.

Data is downloaded from [GeoFabrik](https://download.geofabrik.de/), if you are not sure of what name you need to use, please check there.


# Geocoordinates

CARLA parses a georeference from metadata provided in the OpenDRIVE header that enables the conversion of a CARLA location given in **X**, **Y** and **Z** coordinates (in meters) into a geocoordinate given in **latitude** (degrees), **longitude** (degrees) and **altitude** (meters). The reverse transformation is also possible from latitude, longitude and altitude into a CARLA coordinate. 

A map projection is used to convert between CARLA coordinates and geocoordinates. This projection is used in the GNSS sensor and the `transform_to_geolocation()` and `geolocation_to_transform()` methods of the [Map object](python_api.md#carlamap). The type of projection, a reference ellipsoid along with the geolocation of the center of the map (i.e. X=Y=0) and other parameters associated with the projection are defined in the OpenDRIVE header information in the `<geoReference>` tag in a PROJ.4-style string.

The following is an example of a `<geoReference>` tag provided in an OpenDRIVE file header. Details on the meaning of each parameter are given in the following sections:

```xml
<geoReference><![CDATA[+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84]]></geoReference>
```

The georeference defined in the OpenDRIVE and geoprojection can be queried through the CARLA Python API using the `get_georeference()` and `get_geoprojection()` methods of the [Map object](python_api.md#carlamap).

---

## Map projection

CARLA supports 4 different [map projection types](https://en.wikipedia.org/wiki/Map_projection). The projection type is denoted in the `+proj` field of the georeference:

| Map projection | OpenDRIVE header |
|----------------|------------------|
| [Transverse Mercator](https://en.wikipedia.org/wiki/Transverse_Mercator_projection) (default) | `+proj=tmerc` |
| [Universal Transverse Mercator](https://en.wikipedia.org/wiki/Universal_Transverse_Mercator_coordinate_system) | `+proj=utm` |
| [Web Mercator](https://en.wikipedia.org/wiki/Web_Mercator_projection) | `+proj=merc` |
| [Lambert Conformal Conic - 2SP version](https://en.wikipedia.org/wiki/Lambert_conformal_conic_projection) | `+proj=lcc` |

Each map projection requires a specific set of parameters defined in the OpenDRIVE georeference:

### Transverse mercator

| Parameter | Description |
|-----------|-------------|
| `+proj`   | `tmerc` |
| `+ellps`  | [Reference ellipsoid](#reference-ellipsoid) used to model the Earth's shape |
| `+lat_0`  | Latitude value for the origin of the map |
| `+lon_0`  | Longitude value for the origin of the map |
| `k` | Scale factor at origin |
| `+a`  | Semi-major axis of the ellipsoid model |
| `+b`  | Semi-major axis of the ellipsoid model |
| `+f`  | Flattening of the ellipsoid model (a-b)/a |
| `+f_inv`  | Inverse flattening of the ellipsoid model 1/f |
| `x_0` | False Easting offset |
| `y_0` | False Northing offset |

### Universal transverse mercator

| Parameter | Description |
|-----------|-------------|
| `+proj`   | `utm` |
| `+ellps`  | [Reference ellipsoid](#reference-ellipsoid) used to model the Earth's shape |
| `+zone`  | Defines the longitudinal zone for the UTM projection (1-60) |
| `+north`  | Defines the geolocation as in the Northern hemisphere (default) |
| `+south` | Defines the geolocation as in the Southern hemisphere |
| `+a`  | Semi-major axis of the ellipsoid model |
| `+b`  | Semi-major axis of the ellipsoid model |
| `+f`  | Flattening of the ellipsoid model (a-b)/a |
| `+f_inv`  | Inverse flattening of the ellipsoid model 1/f |

### Web mercator

The Web mercator assumes a spherical Earth, therefore only the **a** parameter, the semi-major axis of the ellipsoid is used. 

| Parameter | Description |
|-----------|-------------|
| `+proj`   | `merc` |
| `+ellps`  | [Reference ellipsoid](#reference-ellipsoid) used to model the Earth's shape |
| `+a`  | Semi-major axis of the ellipsoid model |

### Lambert conformal conic 2SP

| Parameter | Description |
|-----------|-------------|
| `+proj`   | `lcc` |
| `+ellps`  | [Reference ellipsoid](#reference-ellipsoid) used to model the Earth's shape |
| `+lat_0`  | Latitude value for the origin of the map |
| `+lat_1`  | Latitude value for the upper latitude limit |
| `+lat_2`  | Latitude value for the lower latitude limit |
| `+lon_0`  | Longitude value for the origin of the map |
| `+a`  | Semi-major axis of the ellipsoid model |
| `+b`  | Semi-major axis of the ellipsoid model |
| `+f`  | Flattening of the ellipsoid model (a-b)/a |
| `+f_inv`  | Inverse flattening of the ellipsoid model 1/f |
| `x_0` | False Easting offset |
| `y_0` | False Northing offset |

---

## Reference ellipsoid

The reference ellipsoid is the ellipsoid model used to model the Earth's surface. Since the Earth is closer to ellipsoidal than spherical a simple spherical model may cause inaccuracies which are not acceptable for certain applications. The reference ellipsoid can be defined using several parameters:

| Parameter | Description |
|-----------|-------------|
| __a__     | Semi major axis |
| __b__     | Semi minor axis |
| __f__     | Flattening (a-b)/a |
| __f_inv__     | Inverse flattening 1/f |

CARLA supports numerous commonly used reference ellipsoids. The type of ellipsoid to use is defined in the `+ellps` field of the `<geoReference>` tag:

| Ref. ellipsoid | __a__ | __f_inv__ |
|----------------|-------|-----------|
| `wgs84` (default) | 6378137.0 | 298.257223563 |
| `grs80`  | 6378137.0 | 298.257222101 |
| `intl`   | 6378388.0 | 297.0 |
| `bessel` | 6377397.155 | 299.1528128 |
| `clrk66` | 6378206.4 | 294.9786982138 |
| `airy`   | 6377563.396 | 299.3249646 |
| `wgs72`  | 6378135.0 | 298.26 |
| `wgs66`  | 6378145.0 | 298.25 |
| `sphere` | 6370997.0 | inf |

If a reference ellipsoid is defined in the georeference tag and then parameters are also given, the given parameters will override those for the chosen reference ellipsoid. 

---

## Geocoordinates and the Python API

The Python API provides the facility to convert between CARLA coordinates and geocoordinates and also to define projections with custom parameters. The CARLA [Map object](python_api.md#carlamap) maintains the information about the georeference gathered from the OpenDRIVE header and provides methods for conversion:

```py
# Retrieve the map object
carla_map = world.get_map()

# Convert a carla.Location to a carla.Geolocation
geolocation = carla_map.transform_to_geolocation(carla.Location(10,10,1))

# Convert a carla.Geolocation to a carla.Location
geolocation = carla.GeoLocation(latitude=41.400779, longitude=2.188103, altitude=0)
location = carla_map.geolocation_to_transform(geolocation)
```

The Map object provides methods to inspect the current georeference and geoprojection:

```py
# Get the georeference
carla_map.get_georeference() # returns a carla.GeoLocation

# Get the geoprojection
carla_map.get_geoprojection() # returns a carla.GeoProjection
```

### Custom projection

If an alternative georeference is desired, different from that provided in the OpenDRIVE metadata, one can be created using additional API objects.

To define a custom ellipsoid use the `carla.GeoEllipsoid` object:

```py
geoellipsoid = carla.GeoEllipsoid(a=6378137.0, f_inv=297.0)
```

Then to define a projection, use the relevant GeoProjection object:

| Projection type | CARLA object |
|-----------------|--------------|
| Transverse Mercator | carla.GeoProjectionTM |
| Universal Transverse Mercator | carla.GeoProjectionUTM |
| Web Mercator | carla.GeoProjectionWebMerc |
| Lambert Conformal Conic 2SP | carla.GeoProjectionLCC2SP |

```py
geoprojection = carla.GeoProjectionTM(lat_0=41.400779, lon_0=2.188103, k=1.0, ellps=geoellipsoid)
```

This geoprojection can then be used to transform the coordinates using the geotransform methods:

```py
# Retrieve the map object
carla_map = world.get_map()

# Convert a carla.Location to a carla.Geolocation
geolocation = carla_map.transform_to_geolocation(carla.Location(10,10,1), geoprojection)

# Convert a carla.Geolocation to a carla.Location
geolocation = carla.GeoLocation(latitude=41.400779, longitude=2.188103, altitude=0)
location = carla_map.geolocation_to_transform(geolocation, geoprojection)
```








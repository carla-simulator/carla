# Geographic coordinates

CARLA converts between a CARLA location, **X**, **Y** and **Z** in meters, and a geographic coordinate, **latitude** and **longitude** in degrees and **altitude** in meters. It converts through a map projection read from the `<geoReference>` tag in the map's OpenDRIVE header. The [GNSS sensor](ref_sensors.md#gnss-sensor) and the `transform_to_geolocation()` and `geolocation_to_transform()` methods of [`carla.Map`](python_api.md#carla.Map) all use it.

- [__The geoReference tag__](#the-georeference-tag)
- [__Map projections__](#map-projections)
- [__Reference ellipsoid__](#reference-ellipsoid)
- [__Geographic coordinates in the Python API__](#geographic-coordinates-in-the-python-api)

---

## The geoReference tag

The `<geoReference>` tag holds a [PROJ](https://proj.org/)-style string. It names the projection type, the reference ellipsoid and the projection's parameters:

```xml
<header>
    <geoReference><![CDATA[+proj=tmerc +lat_0=41.400779 +lon_0=2.188103 +k=1 +x_0=0 +y_0=0 +ellps=WGS84]]></geoReference>
</header>
```

CARLA reads only the parameters listed on this page and ignores any other, such as `+units`, `+geoidgrids` or `+no_defs`. Altitude is not corrected by a geoid: it is the location's **Z** unchanged, and the reverse.

!!! Note
    A map whose `geoReference` has no `+proj` falls back to a transverse Mercator projection centred on latitude 0, longitude 0, and logs a warning. It keeps the ellipsoid but ignores every other parameter, `+lat_0` and `+lon_0` included. A `+proj` CARLA does not implement falls back the same way, logged only at debug level.

---

## Map projections

CARLA implements four [map projections](https://en.wikipedia.org/wiki/Map_projection), selected by the `+proj` field:

| Map projection | `+proj` | Python object | Georeference returned by `get_georeference()` |
| -------------- | ------- | ------------- | --------------------------------------------- |
| [Transverse Mercator](https://en.wikipedia.org/wiki/Transverse_Mercator_projection) (default) | `tmerc` | [`carla.GeoProjectionTM`](python_api.md#carla.GeoProjectionTM) | `+lat_0`, `+lon_0` |
| [Universal Transverse Mercator](https://en.wikipedia.org/wiki/Universal_Transverse_Mercator_coordinate_system) | `utm` | [`carla.GeoProjectionUTM`](python_api.md#carla.GeoProjectionUTM) | latitude 0, the zone's central meridian (longitude 0 without `+zone`) |
| [Web Mercator](https://en.wikipedia.org/wiki/Web_Mercator_projection) | `merc` | [`carla.GeoProjectionWebMerc`](python_api.md#carla.GeoProjectionWebMerc) | latitude 0, longitude 0 |
| [Lambert Conformal Conic, 2SP](https://en.wikipedia.org/wiki/Lambert_conformal_conic_projection) | `lcc` | [`carla.GeoProjectionLCC2SP`](python_api.md#carla.GeoProjectionLCC2SP) | `+lat_0`, `+lon_0` |

Every projection also takes the [ellipsoid parameters](#reference-ellipsoid). Parameters left out take the default shown.

### Transverse Mercator

| Parameter | Default | Description |
| --------- | ------- | ----------- |
| `+lat_0` | 0 | Latitude of the projection origin, in degrees |
| `+lon_0` | 0 | Longitude of the projection origin, in degrees |
| `+k` | 1 | Scale factor at the origin |
| `+x_0` | 0 | False easting, in meters |
| `+y_0` | 0 | False northing, in meters |

### Universal Transverse Mercator

| Parameter | Default | Description |
| --------- | ------- | ----------- |
| `+zone` | 31 | Longitudinal zone, 1 to 60. A missing zone logs a warning. |
| `+south` | absent | The map is in the southern hemisphere. Without it the map is in the northern one; `+north` is accepted and has no effect. |

UTM fixes the rest: a scale factor of 0.9996, a false easting of 500000 m and a false northing of 0 m in the north or 10000000 m in the south.

UTM is the only projection that applies the OpenDRIVE header's `<offset>` element, which moves and rotates the map's coordinates before they are projected:

```xml
<header>
    <geoReference><![CDATA[+proj=utm +zone=32 +ellps=WGS84]]></geoReference>
    <offset x="691000.0" y="5335000.0" z="0.0" hdg="0.0"/>
</header>
```

`x`, `y` and `z` are in meters and `hdg` in radians; a missing attribute is 0. In Python the offset is [`carla.GeoOffsetTransform`](python_api.md#carla.GeoOffsetTransform), held in the `offset` attribute of `carla.GeoProjectionUTM`, or `None` when the header has none.

### Web Mercator

Web Mercator models the Earth as a sphere of radius `+a`, so it ignores the flattening and takes no other parameter.

### Lambert Conformal Conic, 2SP

| Parameter | Default | Description |
| --------- | ------- | ----------- |
| `+lat_0` | 0 | Latitude of the projection origin, in degrees |
| `+lon_0` | 0 | Longitude of the projection origin, in degrees |
| `+lat_1` | 0 | First standard parallel, in degrees. A missing value logs a warning. |
| `+lat_2` | 0 | Second standard parallel, in degrees. A missing value logs a warning. |
| `+x_0` | 0 | False easting, in meters |
| `+y_0` | 0 | False northing, in meters |

---

## Reference ellipsoid

The reference ellipsoid models the shape of the Earth, which is closer to an ellipsoid than to a sphere. `+ellps`, or its alias `+datum`, names one of these, case-insensitively:

| `+ellps` | Semi-major axis __a__ (m) | Inverse flattening __1/f__ |
| -------- | ------------------------- | -------------------------- |
| `wgs84` (default) | 6378137.0 | 298.257223563 |
| `grs80` | 6378137.0 | 298.257222101 |
| `intl` | 6378388.0 | 297.0 |
| `bessel` | 6377397.155 | 299.1528128 |
| `clrk66` | 6378206.4 | 294.9786982138 |
| `airy` | 6377563.396 | 299.3249646 |
| `wgs72` | 6378135.0 | 298.26 |
| `wgs66` | 6378145.0 | 298.25 |
| `sphere` | 6370997.0 | inf |

These parameters define the ellipsoid directly, or override the values of the named one:

| Parameter | Description |
| --------- | ----------- |
| `+a` | Semi-major axis, in meters |
| `+b` | Semi-minor axis, in meters |
| `+f` | Flattening, (a − b) / a |
| `+rf` | Inverse flattening, 1 / f |

They are applied in the order of the table, so the last one present sets the flattening: `+rf` over `+f` over `+b`. `+a` without a known `+ellps` keeps an infinite inverse flattening, a sphere of radius `+a`. With neither a known `+ellps` nor any of these parameters, the ellipsoid is `wgs84`.

---

## Geographic coordinates in the Python API

[`carla.Map`](python_api.md#carla.Map) converts through the map's own projection:

```py
carla_map = world.get_map()

geolocation = carla_map.transform_to_geolocation(carla.Location(x=10, y=10, z=1))   # carla.GeoLocation

geolocation = carla.GeoLocation(latitude=41.400779, longitude=2.188103, altitude=0)
location = carla_map.geolocation_to_transform(geolocation)                          # carla.Location
```

It also returns the georeference and the projection it parsed:

```py
carla_map.get_georeference()   # carla.GeoLocation, the last column of the projection table
carla_map.get_geoprojection()  # one of the four projection objects, with its parameters
```

### Custom projection

Both conversions take an optional projection, to read or write coordinates in a projection that is not the map's. Build its ellipsoid with [`carla.GeoEllipsoid`](python_api.md#carla.GeoEllipsoid), which takes the semi-major axis and the inverse flattening:

```py
ellipsoid = carla.GeoEllipsoid(a=6378137.0, f_inv=297.0)
projection = carla.GeoProjectionTM(lat_0=41.400779, lon_0=2.188103, k=1.0, ellps=ellipsoid)

geolocation = carla_map.transform_to_geolocation(carla.Location(x=10, y=10, z=1), projection)
location = carla_map.geolocation_to_transform(geolocation, projection)
```

Each projection object takes the parameters of its table above as keyword arguments, without the `+`, and `ellps`.

!!! Important
    `carla.GeoEllipsoid()` defaults to `a=6378137.0` and `f_inv=inf`: a sphere, not WGS84. A projection built without `ellps` uses that sphere, so pass `carla.GeoEllipsoid(a=6378137.0, f_inv=298.257223563)` to match a map on WGS84.

| Python object | Keyword arguments |
| ------------- | ----------------- |
| `carla.GeoProjectionTM` | `lat_0`, `lon_0`, `k`, `x_0`, `y_0`, `ellps` |
| `carla.GeoProjectionUTM` | `zone`, `north`, `ellps`; the `offset` attribute is set after construction |
| `carla.GeoProjectionWebMerc` | `ellps` |
| `carla.GeoProjectionLCC2SP` | `lat_0`, `lat_1`, `lat_2`, `lon_0`, `x_0`, `y_0`, `ellps` |

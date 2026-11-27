# Geocoordinates reference

The full description of the projections and parameters is in `Docs/core_geocoordinates.md`. This page keeps what the skill needs and what was measured.

## API

| Call | Returns |
|---|---|
| `map.transform_to_geolocation(location[, projection])` | `carla.GeoLocation` |
| `map.geolocation_to_transform(geolocation[, projection])` | `carla.Location` (not a Transform, despite the name) |
| `map.get_georeference()` | `carla.GeoLocation` of the projection origin |
| `map.get_geoprojection()` | `GeoProjectionTM`, `GeoProjectionUTM`, `GeoProjectionWebMerc` or `GeoProjectionLCC2SP` |

The projection objects expose their parameters (`lat_0`, `lon_0`, `k`, `x_0`, `y_0` on TM), but `GeoProjectionTM` has no `ellps` attribute, so the map's ellipsoid can only be read from the header (`map.to_opendrive()`).

## Projections

| `+proj` | Class | Keyword arguments |
|---|---|---|
| `tmerc` | `carla.GeoProjectionTM` | `lat_0`, `lon_0`, `k`, `x_0`, `y_0`, `ellps` |
| `utm` | `carla.GeoProjectionUTM` | `zone`, `north`, `ellps`; `offset` is set after construction |
| `merc` | `carla.GeoProjectionWebMerc` | `ellps` |
| `lcc` | `carla.GeoProjectionLCC2SP` | `lat_0`, `lat_1`, `lat_2`, `lon_0`, `x_0`, `y_0`, `ellps` |

UTM is the only projection that applies the header's `<offset x y z hdg>`.

## The ellipsoid trap

`carla.GeoEllipsoid()` defaults to `a=6378137.0, f_inv=inf`, a sphere, while the map parser defaults to WGS84. A projection built by hand without `ellps` therefore disagrees with the map. On Town10HD_Opt, at (-45.2, 30.1), a sphere moves the latitude from -0.000272215 to -0.000270393, about 20 cm. `geo.py` builds WGS84 when the PROJ string names no ellipsoid, like the server.

## Map origins

Measured from the `.xodr` headers in the tree:

| Map | `+lat_0` | `+lon_0` |
|---|---|---|
| Town11 | 41.500588 | 2.111878 |
| Town13 | 41.09374957 | -81.59374805 |
| Town15 | 41.50159463 | 2.10376841 |
| Eixample | 41.3925 | 2.166 |
| Mine_01 | 0.00055133 | -0.00136155 |
| every other town | 0 | 0 |

## Measured on ue58-dev ccc52d24d, Town10HD_Opt

- A `sensor.other.gnss` at spawn point 0 read latitude 0.00011496282595682389, longitude 0.0009559538003642711, altitude 0.5999987721443176; `transform_to_geolocation` of the sensor's location gave the same three values to every digit.
- `--proj` with the map's own header string gave the same result as the map's projection.
- Round trips were under 0.1 mm for TM, UTM, LCC and Web Mercator.

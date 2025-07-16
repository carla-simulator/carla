# SPDX-FileCopyrightText: © 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
Projection and Coordinate Transformation Functions Module

This module provides functions for coordinate system transformations between different
reference frames used in geospatial and mapping applications. It handles conversions
between GPS coordinates, ECEF (Earth-Centered, Earth-Fixed), and local ENU 
(East-North-Up) coordinate systems.

Key Functions:
- lat_lng_alt_2_ECEF_elipsoidal: Convert GPS coordinates to ECEF using ellipsoidal Earth model
- ecef_2_ENU: Compute transformation matrix from ECEF to local ENU coordinates
- get_t_rig_enu_from_ecef: Align NUREC and OpenDRIVE coordinate systems using georeference data

The module is essential for aligning NUREC reconstructions with OpenDRIVE maps, as they
may have different coordinate system origins and orientations. The transformations ensure
proper spatial alignment between the neural reconstruction and the map data.

Coordinate Systems:
- GPS: Latitude, longitude, altitude (WGS84)
- ECEF: Earth-Centered, Earth-Fixed Cartesian coordinates
- ENU: East-North-Up local Cartesian coordinates
- OpenDRIVE: Local coordinate system defined by map georeference

The functions support both spherical and ellipsoidal Earth models, with WGS84 ellipsoid
parameters for high accuracy in large-scale scenarios.
"""

import numpy as np
import xml.etree.ElementTree as ET
import math

def lat_lng_alt_2_ECEF_elipsoidal(lat_lng_alt: np.ndarray, a: float, b: float) -> np.ndarray:
    """Converts the GPS (lat,long, alt) coordinates to the ECEF ones based on the ellipsoidal earth model
    Args:
        lat_lng_alt (np.array): latitude, longitude and altitude coordinate (in degrees and meters) [n,3]
        a (float/double): Semi-major axis of the ellipsoid
        b (float/double): Semi-minor axis of the ellipsoid
    Out:
        (np.array): ECEF coordinates[n,3]
    """

    phi = np.deg2rad(lat_lng_alt[:, 0])
    gamma = np.deg2rad(lat_lng_alt[:, 1])

    cos_phi = np.cos(phi)
    sin_phi = np.sin(phi)
    cos_gamma = np.cos(gamma)
    sin_gamma = np.sin(gamma)
    e_square = (a * a - b * b) / (a * a)

    N = a / np.sqrt(1 - e_square * sin_phi * sin_phi)

    x = (N + lat_lng_alt[:, 2]) * cos_phi * cos_gamma
    y = (N + lat_lng_alt[:, 2]) * cos_phi * sin_gamma
    z = (N * (b * b) / (a * a) + lat_lng_alt[:, 2]) * sin_phi

    return np.concatenate([x[:, None], y[:, None], z[:, None]], axis=1)


def ecef_2_ENU(loc_ref_point: np.ndarray, earth_model: str = "WGS84") -> np.ndarray:
    """
    Compute the transformation matrix that transforms points from the ECEF to a local ENU coordinate frame
    Args:
        loc_ref_point: GPS coordinates of the local reference point of the map [1,3]
        earth_model: earth model used for conversion (spheric will be unaccurate when maps are large)
    Out:
        T_ecef_enu: transformation matrix from ECEF to ENU [4,4]
    """

    # initialize the transformation to identity
    T_ecef_enu = np.eye(4)

    if earth_model == "WGS84":
        a = 6378137.0
        flattening = 1.0 / 298.257223563
        b = a * (1.0 - flattening)
        translation = lat_lng_alt_2_ECEF_elipsoidal(loc_ref_point, a, b).reshape(3, 1)

    elif earth_model == "sphere":
        earth_radius = 6378137.0  # Earth radius in meters
        z_dir = np.concatenate(
            [
                (np.sin(loc_ref_point[1]) * np.cos(loc_ref_point[0]))[:, None],
                (np.sin(loc_ref_point[1]) * np.sin(loc_ref_point[0]))[:, None],
                (np.cos(loc_ref_point[0]))[:, None],
            ],
            axis=1,
        )

        translation = ((earth_radius + loc_ref_point[:, -1])[:, None] * z_dir).reshape(3, 1)

    else:
        raise ValueError("Selected ellipsoid not implemented!")

    rad_lat = np.deg2rad(loc_ref_point[0, 0])
    rad_lon = np.deg2rad(loc_ref_point[0, 1])
    rotation_matrix = np.array(
        [
            [-np.sin(rad_lon), np.cos(rad_lon), 0],
            [-np.sin(rad_lat) * np.cos(rad_lon), -np.sin(rad_lat) * np.sin(rad_lon), np.cos(rad_lat)],
            [np.cos(rad_lat) * np.cos(rad_lon), np.cos(rad_lat) * np.sin(rad_lon), np.sin(rad_lat)],
        ]
    )
    T_ecef_enu[:3, :3] = rotation_matrix

    T_ecef_enu[:3, 3:4] = -T_ecef_enu[:3, :3] @ translation

    return T_ecef_enu


def get_t_rig_enu_from_ecef(t_rig_ecef: np.ndarray, xodr_data: str) -> np.ndarray:
    """
    Compute the transformation matrix from a rig's ECEF (Earth-Centered, Earth-Fixed) coordinates
    to a local ENU (East-North-Up) coordinate frame using georeference data from an OpenDRIVE (xodr) file.

    This function is essential for aligning the coordinate systems used by NUREC and the OpenDRIVE map.
    The NUREC system and the OpenDRIVE map may have different origins and orientations, so this function
    calculates the necessary transformation to align them.

    The process involves:
    1. Parsing the OpenDRIVE file to extract georeference information, which includes the latitude,
       longitude, and altitude of the map's origin.
    2. Using this georeference data to compute a transformation matrix that converts ECEF coordinates
       to the ENU frame defined by the OpenDRIVE map.
    3. Applying this transformation to the input ECEF transformation matrix (t_rig_ecef) to obtain
       the final transformation matrix that aligns the rig's coordinates with the map's ENU frame.

    Args:
        t_rig_ecef (np.ndarray): 4x4 transformation matrix from the rig to ECEF coordinates.
        xodr_data (str): XML string containing OpenDRIVE map data with georeference information.

    Returns:
        np.ndarray: 4x4 transformation matrix from the rig's ECEF coordinates to the map's ENU coordinates.
                    If georeference information cannot be parsed, returns an identity matrix.
    """
    tree = ET.fromstring(xodr_data)
    geo_references = tree.findall(".//geoReference")

    if len(geo_references) < 1:
        return np.identity(4)
    
    geo_reference = geo_references[0]

    # Check if geo_reference.text is None
    if geo_reference.text is None:
        print("geoReference element has no text content!")
        return np.identity(4)

    proj_parts = geo_reference.text.split(" ")

    lats = [float(lat[7:]) for lat in proj_parts if (lat.find("+lat_0") != -1)]
    lons = [float(lon[7:]) for lon in proj_parts if (lon.find("+lon_0") != -1)]
    alts = [float(alt[8:]) for alt in proj_parts if (alt.find("+=alt_0") != -1)]

    if len(lats) != 1 or len(lons) != 1 or len(alts) != 1:
        print("Unable to parse proj string! {}".format(geo_reference.text))
        return np.identity(4)
    
    lat = lats[0]
    lon = lons[0]
    alt = alts[0]

    lat_long_alt = np.array([lat, lon, alt]).reshape(1, 3)

    t_ecef_enu = ecef_2_ENU(lat_long_alt, earth_model="WGS84")

    t_rig_enu =t_ecef_enu @ t_rig_ecef
    
    # check if translation is larger than 10 km
    if math.sqrt(t_rig_enu[0, 3]**2 + t_rig_enu[1, 3]**2 + t_rig_enu[2, 3]**2) > 10000:
        print("ERROR:Translation is larger than 10 km! This is exceptionally large and will likely cause issues.")
        print(f"t_rig_enu: {t_rig_enu}")
        return np.identity(4)

    return t_rig_enu
# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
Blueprint Library Module

This module provides functionality for managing CARLA vehicle and pedestrian blueprints,
including finding the best-fit blueprint based on dimensions and handling coordinate
transformations between bounding box centers and vehicle reference points.

The BlueprintLibrary class loads blueprint size data from JSON files and provides
methods to:
- Find the best matching blueprint for given dimensions
- Apply coordinate offsets between bounding box center and rear axle center
- Handle transformations for accurate vehicle positioning in CARLA
"""

import json
import numpy as np
from dataclasses import dataclass
from typing import List, Union, Optional, Tuple, Any

# create dataclass blueprint
@dataclass
class Blueprint:
    """
    Represents a CARLA blueprint with its dimensions and coordinate offset.
    
    Attributes:
        id (str): Blueprint identifier (e.g., "vehicle.tesla.model3")
        dimensions (List[float]): [length, width, height] in meters
        offset (List[float]): [x, y, z] offset from bounding box center to rear axle center
    """
    id: str
    dimensions: List[float]
    offset: List[float]

class BlueprintLibrary:
    """
    Manages CARLA blueprint data for vehicles and pedestrians.
    
    This class loads blueprint size and offset information from JSON files and provides
    methods to find the best-fit blueprint for given dimensions. It also handles
    coordinate transformations between bounding box centers and vehicle reference points
    (typically the rear axle center for vehicles).
    
    The blueprint data includes:
    - Vehicle blueprints with dimensions and rear axle offsets
    - Pedestrian blueprints with dimensions
    - Methods to apply coordinate transformations for accurate positioning
    """
    def __init__(self) -> None:
        """
        Initialize the blueprint library by loading blueprint data from JSON files.
        
        Loads:
        - blueprint_sizes_vehicle.json: Vehicle blueprint data
        - blueprint_sizes_walker.json: Pedestrian blueprint data
        
        Also creates a lookup table for blueprint offsets for faster access.
        """
        with open("blueprint_sizes_vehicle.json", "r") as f:
            self.blueprint_sizes_vehicle = json.load(f)
        with open("blueprint_sizes_walker.json", "r") as f:
            self.blueprint_sizes_walker = json.load(f)
        # Store offsets for faster lookup
        self.blueprint_offsets = {}
        for blueprint in self.blueprint_sizes_vehicle:
            if len(blueprint) >= 3:  # Make sure the offset is present
                self.blueprint_offsets[blueprint[0]] = np.array(blueprint[2])

    def get_best_fit_blueprint(self, dimensions: Union[List[float], np.ndarray], vehicle: bool = True) -> Blueprint:
        """
        Find the best-fit blueprint for given dimensions using least squares error.
        
        Args:
            dimensions (Union[List[float], np.ndarray]): Target dimensions [length, width, height] in meters
            vehicle (bool): If True, search vehicle blueprints; if False, search pedestrian blueprints
            
        Returns:
            Blueprint: The blueprint with dimensions closest to the target dimensions
        """
        if vehicle:
            blueprint_sizes = self.blueprint_sizes_vehicle
        else:
            blueprint_sizes = self.blueprint_sizes_walker
        best_fit_blueprint: Blueprint = Blueprint(id="", dimensions=[], offset=[])
        best_squared_error = float("inf")
        for blueprint in blueprint_sizes:
            delta = np.array(blueprint[1]) - np.array(dimensions) / 2
            squared_error = np.sum(delta**2)
            if squared_error < best_squared_error:
                best_squared_error = squared_error
                best_fit_blueprint = Blueprint(id=blueprint[0], dimensions=blueprint[1], offset=blueprint[2])
        return best_fit_blueprint

    def get_offset(self, blueprint_id: str) -> np.ndarray:
        """
        Get the offset from the center of the bounding box to the rear axle center.
        
        Args:
            blueprint_id (str): The blueprint identifier
            
        Returns:
            np.ndarray: [x, y, z] offset vector, or zeros if blueprint not found
        """
        if blueprint_id in self.blueprint_offsets:
            return self.blueprint_offsets[blueprint_id]
        return np.zeros(3)  # Default offset if not found

    def apply_offset_to_pose(self, pose_matrix: np.ndarray, blueprint_id: str, inverse: bool = False) -> np.ndarray:
        """
        Apply the offset between bounding box center and rear axle to a pose.

        Args:
            pose_matrix (np.ndarray): 4x4 transformation matrix
            blueprint_id (str): The blueprint ID to get the offset for
            inverse (bool): If True, applies the inverse offset (from rear axle to bounding box center)

        Returns:
            np.ndarray: A 4x4 transformation matrix with the offset applied
        """
        offset = self.get_offset(blueprint_id)

        # Create offset matrix
        offset_matrix = np.eye(4)
        if inverse:
            offset_matrix[:3, 3] = -offset  # Negate for inverse
        else:
            offset_matrix[:3, 3] = offset

        # Apply offset
        return pose_matrix @ offset_matrix
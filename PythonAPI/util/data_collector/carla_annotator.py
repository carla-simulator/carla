#!/usr/bin/env python3
#
# Copyright (c) 2022 Intel Corporation.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
#

from PIL import Image
import json
import numpy as np
import os
import pathlib
import datetime
import argparse
import fnmatch
import hashlib
import cv2
import imageio
from tqdm import tqdm
from pycococreatortools import pycococreatortools
import pycocotools.mask as rletools
import logging
import coloredlogs
from contextlib import nullcontext
from scipy import ndimage

logger = logging.getLogger(__name__)
coloredlogs.install(level=logging.INFO)

INFO = {
    "description": "Carla Dataset",
    "url": "https://github.com/carla-simulator/carla",
    "version": "0.9.12",
    "year": 2021,
    "contributor": "",
    "date_created": datetime.datetime.utcnow().isoformat(" "),
}

LICENSES = [{"id": 1, "name": "", "url": ""}]

# Define Carla semantic categories - as per Carla v0.9.13
carla_labels = [
    "Unlabeled",
    "Building",
    "Fence",
    "Other",
    "Pedestrian",
    "Pole",
    "RoadLine",
    "Road",
    "Sidewalk",
    "Vegetation",
    "Vehicle",
    "Wall",
    "TrafficSign",
    "Sky",
    "Ground",
    "Bridge",
    "RailTrack",
    "GuardRail",
    "TrafficLight",
    "Static",
    "Dynamic",
    "Water",
    "Terrain",
]


def extract_masks(im, category_ids, combine_twowheeled=False, twowheeled_as_pedestrian=False, closing=False):
    """Extract binary masks from CARLA's instance segmentation sensor outputs.

    Parameters
    ----------
    im : numpy.ndarray
        3-channel instance segmentation image.
    category_ids : list(int)
        The category ID of interests.
    combine_twowheeled: bool
        If True, combine the driver and the underlying 2-wheeled vehicle into one single instance with same semantic label (default label: 'Vehilce'); Otherwise, separate them into two instances (the driver labeled as 'Pedestrian' and 2-wheeled vehicle labeled as 'Vehilce')
    twowheeled_as_pedestrian: bool
        If True, label the combined 2-wheeled instance as 'Pedestrian'; Otherwise label it as 'Vehicle'
    closing: bool
        If True, fill holes in binary mask of every object
    Returns
    -------
    masks: List[(int, np.array, int)]
        A list of tuples (category_id, binary_mask, is_crowd)
            category_id: Category ID as in CARLA dataset.
            binary_mask: The binary mask extracted for the respective object ID
            is_crowd: The value for is_crowd (TODO: Support partial occlusion)
    """
    masks = []
    img_label = im[:, :, 0]

    # R = Category id, G = first byte of Object ID, B = second byte of object ID
    # Merge G and B values to resolve Object ID and create 2D array with only Object IDs
    obj_ids = (im[:, :, 2].astype(np.uint32) << 8) + im[:, :, 1].astype(np.uint32)

    # List of all unique Object IDs
    all_obj_ids = list(np.unique(obj_ids))

    for obj_id in all_obj_ids:
        labels = np.unique(img_label[np.where(obj_ids == obj_id)])
        labels = [label for label in labels if label in category_ids]

        if not labels:
            continue

        is_crowd = 0

        two_wheeled_labels = [carla_labels.index("Vehicle"), carla_labels.index("Pedestrian")]
        if combine_twowheeled and sorted(labels) == sorted(two_wheeled_labels):
            # If there are two semantic labels ("Vehicle" and "Pedestrian") mapping to the same object id, it is a 2-wheeled vehicle.
            # So here generate only one mask and set same semantic label. Remove the object id from the id list to avoid appending the mask duplicatedly.
            extract_obj_binary = np.zeros_like(img_label)
            extract_obj_binary[np.where((obj_ids == obj_id))] = 1
            if twowheeled_as_pedestrian:
                label = carla_labels.index("Pedestrian")
            else:
                label = carla_labels.index("Vehicle")
            if np.sum(extract_obj_binary) > 0:
                masks.append((label, extract_obj_binary, is_crowd, obj_id))

            continue

        for label in labels:
            extract_obj_binary = np.zeros_like(img_label)
            extract_obj_binary[np.where((img_label == label) & (obj_ids == obj_id))] = 1
            if closing:
                extract_obj_binary = ndimage.binary_fill_holes(extract_obj_binary).astype(np.uint8)

            if np.sum(extract_obj_binary) > 0:
                contours, _ = cv2.findContours(extract_obj_binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
                # TBD: Support partial occlusion using IoU of segmentation mask area and 3d bounding boxes from carla projected into 2d space
                if len(contours) > 1:
                    is_crowd = 1
                if label == carla_labels.index("TrafficLight"):
                    # Treat TrafficLight differently as we want to only extract lights (do not include poles)
                    for contour in contours:
                        # Valid polygons have >=6 coordinates (3 points)
                        if contour.size >= 6:
                            binary_mask = np.zeros_like(img_label)
                            binary_mask = cv2.fillPoly(binary_mask, pts=[contour], color=(1, 1, 1))
                            masks.append((label, binary_mask, is_crowd, obj_id))
                else:
                    masks.append((label, extract_obj_binary, is_crowd, obj_id))

    return masks


def convert_instancemaps_to_mots_format(
    dir_pairs,
    labels,
    interval=5,
    format="mots_png",
    out_dir="instances",
    combine_twowheeled=False,
    twowheeled_as_pedestrian=False,
    closing=True,
):
    """
    Create COCO format annotations for CARLA dataset

    Parameters
    ----------
    dir_pairs: List(Tuple)
        The directory mappings of where Instance segmentation images and sensor images are stored in raw format
    labels: List(str)
        A list of CARLA labels to generate annotations
    interval: int
        The interval for generating annotations
    output: str
        Output file for COCO annotations in json format.
    combine_twowheeled: bool
        If True, combine the driver and the underlying 2-wheeled vehicle into one single instance with same semantic label (default label: 'Vehilce'); Otherwise, separate them into two instances (the driver labeled as 'Pedestrian' and 2-wheeled vehicle labeled as 'Vehilce')
    twowheeled_as_pedestrian: bool
        If True, label the combined 2-wheeled instance as 'Pedestrian'; Otherwise label it as 'Vehicle'
    closing: bool
        If True, fill holes in binary masks
    """

    # TBD: Shorten this if possible
    # Currently annotating Pedestrian, Vehicle, TrafficLight
    category_ids = []
    for label in labels:
        if label not in carla_labels:
            print(f"Skip label {label} as it is not a CARLA semantic label")
            continue
        category_ids.append(carla_labels.index(label))

    categories = [
        {"supercategory": carla_labels[category], "id": i + 1, "name": carla_labels[category]}
        for i, category in enumerate(category_ids)
    ]

    annot_id = 1

    for dir_pair in tqdm(dir_pairs, desc="Annotation progress"):

        if not os.path.exists(out_dir):
            logger.info("Making dir: " + out_dir)
            pathlib.Path(out_dir).mkdir(parents=True, exist_ok=True)

        labels_path = os.path.join(out_dir, "labels.txt")

        dict_annot = {}
        obj_id_count = 1
        labels_dir, camera_dir = dir_pair
        logger.info(f"\nGenerating annotations for {camera_dir}")
        # Check path to RGB and instance segmentation image directories
        for dir in [camera_dir, labels_dir]:
            if not os.path.exists(dir):
                raise FileNotFoundError("{0} folder does not exist!".format(dir))

        # Only select the RGB images with corresponding instance segmentation images.
        fnames = []
        label_fnames = os.listdir(labels_dir)
        for fname in os.listdir(camera_dir):
            if fname in label_fnames:
                fnames.append(fname)

        _context_manager = open(os.path.join(out_dir, "instances.txt"), "w+") if format == "mots_txt" else nullcontext()

        with _context_manager as ann_file:
            for fname in tqdm(fnames, desc="Frames annotated in directory"):
                im_id = int(fname.split(".")[0])
                # Generate annotation every "interval" frames
                if im_id % interval == 0:
                    fpath_label = os.path.join(labels_dir, fname)
                    im = Image.open(fpath_label)
                    labels_mat = np.array(im)
                    height, width, _ = labels_mat.shape
                    masks = extract_masks(labels_mat, category_ids, combine_twowheeled,
                                          twowheeled_as_pedestrian, closing)

                    if format == "mots_png":
                        dest_mots_gt = os.path.join(out_dir, fname)
                        # Create image annotations with 16 bit png channel
                        mots_png_output = np.zeros((height, width), dtype=np.uint16)

                    for mask in masks:
                        class_id = mask[0]
                        binary_mask = mask[1]
                        is_crowd = mask[2]
                        category_id = [
                            category["id"] for category in categories if category["name"] == carla_labels[class_id]
                        ]
                        category_info = {"id": category_id[0], "is_crowd": is_crowd}
                        # We have not implemented a way to enable is_crowd for cases where there is occlusion and only one part of an object is visible.
                        annotation_info = pycococreatortools.create_annotation_info(
                            annot_id, im_id, category_info, binary_mask, (width, height)
                        )

                        if annotation_info is not None:
                            # Map UE object IDs to IDs starting from 1
                            if mask[3] not in dict_annot.keys():
                                dict_annot[mask[3]] = obj_id_count
                                obj_id_count += 1

                            object_id = int(category_id[0]) * 1000 + int(dict_annot[mask[3]])
                            if format == "mots_png":
                                idx = np.where(binary_mask == 1)
                                mots_png_output[idx] = object_id
                            else:
                                rle = rletools.encode(np.asfortranarray(binary_mask))["counts"]
                                logger.debug("RLE: " + str(rle, "utf-8"))
                                ann_str = (
                                    str(im_id)
                                    + " "
                                    + str(object_id)
                                    + " "
                                    + str(class_id)
                                    + " "
                                    + str(height)
                                    + " "
                                    + str(width)
                                    + " "
                                    + str(rle, "utf-8")
                                    + "\n"
                                )
                                ann_file.write(ann_str)

                    if format == "mots_png":
                        (unique, counts) = np.unique(mots_png_output, return_counts=True)
                        frequencies = np.asarray((unique, counts)).T
                        logger.debug("frequencies: " + str(frequencies))

                        # Write annotation images to instances/ folder
                        imageio.imwrite(dest_mots_gt, mots_png_output.astype(np.uint16))

        logger.info("All objects annotated dict mapping (UE_ID: instance_id): " + str(dict_annot))

        with open(labels_path, "w+") as labels_file:
            for category in categories:
                labels_file.write(category["name"] + "\n")


def convert_instancemaps_to_kwcoco_format(
    dataset_dir_name,
    dir_pairs,
    labels,
    interval=5,
    output="out.json",
    combine_twowheeled=False,
    twowheeled_as_pedestrian=False,
    tolerance=2,
):
    """
    Create COCO format annotations for CARLA dataset

    Parameters
    ----------
    dataset_dir_name: str
        The root directory where the data is stored
    dir_pairs: List(Tuple)
        The directory mappings of where Instance segmentation images and sensor images are stored in raw format
    labels: List(str)
        A list of CARLA labels to generate annotations
    interval: int
        The interval for generating annotations
    output: str
        Output file for COCO annotations in json format.
    combine_twowheeled: bool
        If True, combine the driver and the underlying 2-wheeled vehicle into one single instance with same semantic label (default label: 'Vehilce'); Otherwise, separate them into two instances (the driver labeled as 'Pedestrian' and 2-wheeled vehicle labeled as 'Vehilce')
    twowheeled_as_pedestrian: bool
        If True, label the combined 2-wheeled instance as 'Pedestrian'; Otherwise label it as 'Vehicle'
    tolerance: int
        Tolerance as required by skimage.measure.approximate_polygon. Tolerance is the maximum distance from original points of polygon to approximated polygonal chain. If tolerance is 0, the original coordinate array is returned.
    """

    # Currently annotating Pedestrian, Vehicle, TrafficLight
    category_ids = []
    for label in labels:
        if label not in carla_labels:
            print(f"Skip label {label} as it is not a CARLA semantic label")
            continue
        category_ids.append(carla_labels.index(label))

    categories = [
        {"supercategory": carla_labels[category], "id": i + 1, "name": carla_labels[category]}
        for i, category in enumerate(category_ids)
    ]

    annotations = []
    images = []
    annot_id = 1

    for dir_pair in tqdm(dir_pairs, desc="Annotation progress"):
        dict_annot = {}
        obj_id_count = 1
        labels_dir, camera_dir = dir_pair
        print(f"\nGenerating annotations for {camera_dir}")
        # Check path to RGB and instance segmentation image directories
        for dir in [camera_dir, labels_dir]:
            if not os.path.exists(dir):
                raise FileNotFoundError("{0} folder does not exist!".format(dir))

        # Only select the RGB images with corresponding instance segmentation images.
        fnames = []
        label_fnames = os.listdir(labels_dir)
        for fname in os.listdir(camera_dir):
            if fname in label_fnames:
                fnames.append(fname)
        camera_dir_name = camera_dir.split(".")[-1]

        for fname in tqdm(fnames, desc="Frames annotated in directory"):
            im_id = int(fname.split(".")[0])
            # Generate annotation every "interval" frames
            if im_id % interval == 0:
                fpath_label = os.path.join(labels_dir, fname)
                fpath_rgb = os.path.join(camera_dir, fname)
                im = Image.open(fpath_label)
                fpath = fpath_rgb[fpath_rgb.index(dataset_dir_name) + len(dataset_dir_name) + 1:]
                image_id = int(hashlib.sha256(fpath.encode("utf-8")).hexdigest(), 16) % 10**8
                labels_mat = np.array(im)
                height, width, _ = labels_mat.shape

                image_info = pycococreatortools.create_image_info(image_id, fpath, (width, height))
                image_info["video_id"] = str(camera_dir_name)
                image_info["frame_index"] = str(im_id)
                images.append(image_info)

                masks = extract_masks(labels_mat, category_ids, combine_twowheeled, twowheeled_as_pedestrian)

                for mask in masks:
                    class_id = mask[0]
                    binary_mask = mask[1]
                    is_crowd = mask[2]
                    category_id = [
                        category["id"] for category in categories if category["name"] == carla_labels[class_id]
                    ]
                    # We have not implemented a way to enable is_crowd for cases where there is occlusion and only one part of an object is visible.
                    category_info = {"id": category_id[0], "is_crowd": is_crowd}
                    annotation_info = pycococreatortools.create_annotation_info(
                        annot_id, image_id, category_info, binary_mask, (width, height), tolerance=tolerance
                    )

                    if annotation_info is not None:
                        # Map UE object IDs to IDs starting from 1
                        if mask[3] not in dict_annot.keys():
                            dict_annot[mask[3]] = obj_id_count
                            obj_id_count += 1

                        annotation_info["track_id"] = str(dict_annot[mask[3]])
                        annotations.append(annotation_info)

                    annot_id = annot_id + 1

    annotation_obj = {}
    annotation_obj["annotations"] = annotations
    annotation_obj["videos"] = []
    annotation_obj["images"] = images
    annotation_obj["categories"] = categories
    annotation_obj["info"] = INFO
    annotation_obj["licenses"] = LICENSES

    with open(output, "w+") as json_file:
        print(f"Saving output file: {output}")
        json.dump(annotation_obj, json_file, indent=4)


if __name__ == "__main__":

    argparser = argparse.ArgumentParser(description="COCO converter")
    argparser.add_argument("--list", action="store_true", help="List CARLA semantic labels")
    argparser.add_argument(
        "--dataset_parent_dir",
        metavar="dataset_parent_directory",
        default="outputs",
        type=str,
        help="The directory where all the data is stored",
    )
    argparser.add_argument(
        "--sensor_out_dir",
        metavar="sensor_output_directory",
        default="_out",
        type=str,
        help="The directory where the sensor data is stored",
    )
    argparser.add_argument(
        "--output",
        metavar="annotation_output",
        default="out.json",
        type=str,
        help="The output annotation file stored in JSON format",
    )
    argparser.add_argument(
        "--interval",
        metavar="annotation_interval",
        default=1,
        type=int,
        help="Frame interval for annotation"
    )
    argparser.add_argument(
        "--combine_twowheeled",
        action="store_true",
        help="Combine the driver and the underlying vehicle into one single instance with same semantic label (default label: Vehicle)",
    )
    argparser.add_argument(
        "--twowheeled_as_pedestrian",
        action="store_true",
        help="Combine the driver and the underlying vehicle into one single instance with semantic label Pedestrian",
    )
    argparser.add_argument(
        "--labels",
        metavar="annotation_labels",
        nargs="+",
        default=["Pedestrian", "Vehicle", "TrafficLight"],
        type=str,
        help="Labels used to generate annotation.",
    )
    argparser.add_argument(
        "--cameras",
        metavar="cameras_to_annotate",
        nargs="+",
        default=["rgb"],
        type=str,
        help="Types of camera to generate annotations",
    )
    argparser.add_argument(
        "--exclude_dirs",
        metavar="ignored_dataset_subdirectories",
        nargs="*",
        type=str,
        help="Dataset subdirectories to be excluded from annotations",
    )
    argparser.add_argument(
        "--format",
        default="kwcoco",
        type=str,
        help="Annotation format - choose between 'kwcoco', 'mots_txt' or 'mots_png'",
    )
    argparser.add_argument(
        "--binary_fill_holes",
        default=True,
        type=bool,
        help="Close holes in binary mask - using ndimage.binary_fill_holes operation",
    )

    args = argparser.parse_args()

    if args.list:
        print(carla_labels)

    camera_patterns = []
    for camera in args.cameras:
        camera_patterns.append("*sensor.camera." + camera + ".*")

    dir_pairs = []
    recursive_dirs = set(x[0] for x in os.walk(args.dataset_parent_dir))

    if args.exclude_dirs:
        removed = set()
        for excluded_dir in args.exclude_dirs:
            matched = filter(lambda dir: excluded_dir in dir, recursive_dirs)
            removed = removed.union(set(matched))
        recursive_dirs = recursive_dirs.difference(set(removed))

    # Currently there is no information about the relation between spawned sensors. So this tool only annotates images from a CARLA simulation where only one set of sensors are spawned (one RGB sensor and/or one depth sensor, one instance segmentation sensor spawned at the same transform with same sensor attributes).
    # TODO: Save sensor relations in 'carla_data_saver.py' at the simulation time and then retrieve that information when generating annotations.
    for out_dir in recursive_dirs:
        if fnmatch.fnmatch(out_dir, "*/" + args.sensor_out_dir):
            dirs = [x[0] for x in os.walk(out_dir)]
            instance_dir = None
            camera_dir = None
            for subdir in dirs:
                if fnmatch.fnmatch(subdir, "*instance_segmentation*"):
                    instance_dir = subdir
                    break

            for subdir in dirs:
                for camera in camera_patterns:
                    if fnmatch.fnmatch(subdir, camera):
                        camera_dir = subdir
                if instance_dir is not None and camera_dir is not None:
                    dir_pairs.append((instance_dir, camera_dir))
                    camera_dir = None

    if args.format in ["mots_png", "mots_txt"]:
        convert_instancemaps_to_mots_format(
            dir_pairs,
            list(args.labels),
            args.interval,
            format=args.format,
            out_dir=args.output,
            combine_twowheeled=False,
            twowheeled_as_pedestrian=False,
            closing=args.binary_fill_holes
        )
    elif args.format == "kwcoco":
        convert_instancemaps_to_kwcoco_format(
            args.dataset_parent_dir,
            dir_pairs,
            list(args.labels),
            interval=1,
            output=args.output,
            combine_twowheeled=False,
            twowheeled_as_pedestrian=False,
        )

import json
import numpy as np
import pycocotools.mask as rletools
from pycocotools.coco import COCO
import argparse
import os
import cv2
import logging
import coloredlogs
import imageio
import errno
import pathlib
from tqdm import tqdm

logger = logging.getLogger(__name__)
coloredlogs.install(level=logging.INFO)


def kwcoco_to_mots(input_annotations_file, out_dir, format="png"):
    with open(input_annotations_file, "r") as annotations_file:
        labels = json.load(annotations_file)

    coco = COCO(input_annotations_file)
    annotations = labels["annotations"]
    videos = labels["videos"]
    images = labels["images"]
    categories = labels["categories"]
    info = labels["info"]
    licenses = labels["licenses"]

    dir_map_seq = {}
    sequence_id = 1

    for image in images:
        file_path = image["file_name"]
        dir_name, file_name = os.path.split(file_path)
        if dir_name not in dir_map_seq.keys():
            dir_map_seq[dir_name] = (sequence_id, [image])
            sequence_id += 1
        else:
            dir_map_seq[dir_name][1].append(image)

    for dir in tqdm(dir_map_seq.keys(), desc="Annotation progress"):
        labels_path = ""
        logger.info(f"\nGenerating annotations for {dir}")

        for image in tqdm(dir_map_seq[dir][1], desc="Annotating frame in directory"):
            im_id = image["id"]
            im_width = image["width"]
            im_height = image["height"]

            file_path = image["file_name"]
            dir_name, file_name = os.path.split(file_path)

            frame_id = file_name.split(".")[0]  # Extract the frame id to save images in MOTS format

            if not os.path.exists(out_dir):
                logger.info("Making dir: " + out_dir)
                pathlib.Path(out_dir).mkdir(parents=True, exist_ok=True)

            labels_path = os.path.join(out_dir, "labels.txt")

            if format == "txt":

                with open(os.path.join(out_dir, "instances.txt"), "w+") as ann_file:
                    for ann in annotations:
                        if im_id == ann["image_id"]:
                            class_id = ann["category_id"]
                            object_id = int(class_id * 1000) + int(ann["track_id"])

                            mask = coco.annToMask(ann)  # .astype(np.uint16)
                            rle = rletools.encode(np.asfortranarray(mask))["counts"]

                            logger.debug("RLE: " + str(rle, "utf-8"))
                            ann_str = (
                                str(frame_id)
                                + " "
                                + str(object_id)
                                + " "
                                + str(class_id)
                                + " "
                                + str(im_height)
                                + " "
                                + str(im_width)
                                + " "
                                + str(rle, "utf-8")
                                + "\n"
                            )
                            ann_file.write(ann_str)
            else:
                dest_mots_gt = os.path.join(out_dir, file_name)

                # Create image annotations with 16 bit png channel
                mots_png_output = np.zeros((im_height, im_width), dtype=np.uint16)

                for ann in annotations:
                    if im_id == ann["image_id"]:
                        class_id = ann["category_id"]
                        object_id = int(class_id) * 1000 + int(ann["track_id"])
                        mask = coco.annToMask(ann).astype(np.uint16)
                        idx = np.where(mask == 1)
                        mots_png_output[idx] = object_id
                (unique, counts) = np.unique(mots_png_output, return_counts=True)
                frequencies = np.asarray((unique, counts)).T
                logger.debug("frequencies: " + str(frequencies))

                # Write annotation images to instances/ folder
                imageio.imwrite(dest_mots_gt, mots_png_output.astype(np.uint16))

        with open(labels_path, "w+") as labels_file:
            for category in categories:
                labels_file.write(category["name"] + "\n")


if __name__ == "__main__":
    argparser = argparse.ArgumentParser(description="KWCOCO to MOTS converter")
    argparser.add_argument(
        "--input_annotation_file",
        metavar="input_annotation_file",
        default="out_kwcoco_track.json",
        type=str,
        help="The input annotation file in kwcoco format",
    )
    argparser.add_argument(
        "--output_dir",
        metavar="output_dir",
        default="",
        type=str,
        help="The directory where the output annotations will be stored",
    )
    argparser.add_argument(
        "--mots_format",
        metavar="mots_format",
        default="png",
        type=str,
        help="MOTS annotation format - choose between 'png' or 'txt'",
    )

    args = argparser.parse_args()

    kwcoco_to_mots(args.input_annotation_file, args.output_dir, format=args.mots_format)

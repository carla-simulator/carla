#--------------------------------------------
# Script to generate a video from a folder of frames
#--------------------------------------------

from moviepy.video.io import ImageSequenceClip
import glob
from natsort import natsorted
import argparse

def argument_parser():

    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        '--frames',
        required=True,
        help='Folder where frames are stored')
    argparser.add_argument(
        '--video',
        default="video",
        type=str,
        help='Name of the output video')
    argparser.add_argument(
        '--fps',
        default=20,
        type=int,
        help='Frames per second (FPS)')
    
    return argparser.parse_args()

args = argument_parser()

frames_folder = args.frames
video_name = args.video+'.mp4'
fps = args.fps

# Get all frames in folder
image_files = natsorted(glob.glob(frames_folder+"/*"))
# Exclude last frame, which sometimes has not been rendered correctly
image_files = image_files[:-1]

# Create video and save
clip = ImageSequenceClip.ImageSequenceClip(image_files, fps=fps)
clip.write_videofile(video_name)

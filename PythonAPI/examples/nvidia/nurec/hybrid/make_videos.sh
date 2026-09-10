#!/bin/bash
# usage: make_videos.sh RUN_DIR FPS   (expects RUN_DIR/frames/{neural,naive,comp,harm}_NNNN.png)
set -e
R=$1; FPS=${2:-20}; F=$R/frames
ffmpeg -y -loglevel error -framerate $FPS -i $F/harm_%04d.png -c:v libx264 -pix_fmt yuv420p -crf 18 $R/hybrid_final.mp4
ffmpeg -y -loglevel error -framerate $FPS -i $F/neural_%04d.png -c:v libx264 -pix_fmt yuv420p -crf 18 $R/neural_only.mp4
ffmpeg -y -loglevel error -framerate $FPS -i $F/naive_%04d.png -c:v libx264 -pix_fmt yuv420p -crf 18 $R/naive_paste.mp4
# side by side: neural | final  (960x540 each)
ffmpeg -y -loglevel error -framerate $FPS -i $F/neural_%04d.png -framerate $FPS -i $F/harm_%04d.png \
  -filter_complex "[0:v]scale=960:540,drawtext=text='neural scene':x=10:y=10:fontsize=24:fontcolor=yellow[a];[1:v]scale=960:540,drawtext=text='hybrid (CARLA cars + depth + shadow + harmonizer)':x=10:y=10:fontsize=24:fontcolor=yellow[b];[a][b]hstack" \
  -c:v libx264 -pix_fmt yuv420p -crf 18 $R/compare_neural_vs_hybrid.mp4
# naive paste | final
ffmpeg -y -loglevel error -framerate $FPS -i $F/naive_%04d.png -framerate $FPS -i $F/harm_%04d.png \
  -filter_complex "[0:v]scale=960:540,drawtext=text='naive paste':x=10:y=10:fontsize=24:fontcolor=yellow[a];[1:v]scale=960:540,drawtext=text='depth-tested + shadow + harmonizer':x=10:y=10:fontsize=24:fontcolor=yellow[b];[a][b]hstack" \
  -c:v libx264 -pix_fmt yuv420p -crf 18 $R/compare_naive_vs_hybrid.mp4
ls -la $R/*.mp4 | awk '{print $5, $9}'

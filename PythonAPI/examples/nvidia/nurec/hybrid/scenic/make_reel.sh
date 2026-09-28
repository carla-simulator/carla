#!/bin/bash
# Promo reel from hybrid runs: each run's harmonized clip with a caption, 0.6 s cross-fades, 1080p H.264.
# usage: make_reel.sh OUT.mp4 RUN_DIR:"caption" [RUN_DIR:"caption" ...]   (RUN_DIR holds hybrid_temporal.mp4, else hybrid_final.mp4)
# Captions are drawn in the lower-left corner during the first 4 s of each clip (fades out); the last frame holds 0.5 s.
set -e
OUT=$1; shift
FONT=${REEL_FONT:-/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf}
TMP=$(mktemp -d); i=0; INPUTS=(); FILTER=""; LABELS=()
for spec in "$@"; do
  RUN=${spec%%:*}; CAP=${spec#*:}
  SRC=$RUN/hybrid_temporal.mp4; [ -f "$SRC" ] || SRC=$RUN/hybrid_final.mp4
  [ -f "$SRC" ] || { echo "no clip in $RUN" >&2; exit 1; }
  ESC=$(printf '%s' "$CAP" | sed "s/'/\\\\'/g; s/:/\\\\:/g")
  ffmpeg -y -loglevel error -i "$SRC" -vf "drawbox=x=0:y=ih-120:w=iw:h=120:color=black@0.45:t=fill:enable='between(t,0,4)',\
drawtext=fontfile=$FONT:text='$ESC':x=48:y=h-84:fontsize=40:fontcolor=white:alpha='if(lt(t,3.2),1,max(0,1-(t-3.2)/0.8))':enable='between(t,0,4)',tpad=stop_mode=clone:stop_duration=0.5" \
    -c:v libx264 -pix_fmt yuv420p -crf 18 "$TMP/clip$i.mp4"
  INPUTS+=(-i "$TMP/clip$i.mp4"); i=$((i+1))
done
if [ $i -eq 1 ]; then cp "$TMP/clip0.mp4" "$OUT"; rm -rf "$TMP"; echo "$OUT"; exit 0; fi
# chain of xfades: offsets = cumulative durations minus the fades
PREV="[0:v]"; OFF=0; FILTER=""
for ((k=1; k<i; k++)); do
  D=$(ffprobe -v error -show_entries format=duration -of csv=p=0 "$TMP/clip$((k-1)).mp4")
  OFF=$(python3 -c "print(round($OFF + $D - 0.6, 3))")
  LABEL="[v$k]"; [ $k -eq $((i-1)) ] && LABEL="[vout]"
  FILTER="$FILTER${PREV}[$k:v]xfade=transition=fade:duration=0.6:offset=$OFF$LABEL;"
  PREV=$LABEL
done
ffmpeg -y -loglevel error "${INPUTS[@]}" -filter_complex "${FILTER%;}" -map "[vout]" -c:v libx264 -pix_fmt yuv420p -crf 18 "$OUT"
rm -rf "$TMP"; echo "$OUT"

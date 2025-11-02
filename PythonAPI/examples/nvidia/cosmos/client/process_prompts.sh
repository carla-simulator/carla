#!/bin/bash

# Define the directory containing the TOML files
TOML_DIR="./example_data/prompts"

# Define the list of user-defined seeds
SEEDS=(512 1024 2048 31858)

# Endpoint (optional positional argument; defaults to local server)
ENDPOINT="${1:-http://127.0.0.1:8080}"

# Define the input files
INPUT_VIDEO="example_data/artifacts/rgb.mp4"
EDGE_VIDEO="example_data/artifacts/edges.mp4"
DEPTH_VIDEO="example_data/artifacts/depth.mp4"
SEG_VIDEO="example_data/artifacts/semantic_segmentation.mp4"

# Gather and sort TOML files into an array
TOML_FILES=( $(find "$TOML_DIR" -maxdepth 1 -type f -name '*.toml' | sort) )

# Handle no-file case
if [[ ${#TOML_FILES[@]} -eq 0 ]]; then
  echo "No TOML files found in $TOML_DIR"
  exit 0
fi

# Iterate over sorted TOML files
for toml_file in "${TOML_FILES[@]}"; do
  # Get the base name of the TOML file without the directory and extension
  base_name=$(basename "$toml_file" .toml)
  
  # Iterate over the list of seeds
  for seed in "${SEEDS[@]}"; do
    # Define the output directory
    output_name="outputs/${base_name}_seed_${seed}.mp4"
         
    # Run the python script and capture any errors
    if ! python cosmos_client.py "$ENDPOINT" "$toml_file" --input-video "$INPUT_VIDEO" --edge-video "$EDGE_VIDEO" --seg-video "$SEG_VIDEO" --seed "$seed" --output "$output_name"; then
      echo "Error processing $toml_file with seed $seed"
    fi
  done
done

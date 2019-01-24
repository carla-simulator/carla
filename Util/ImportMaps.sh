#! /bin/bash

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

source $(dirname "$0")/Environment.sh

if [ ! -d "${UE4_ROOT}" ]; then
  fatal_error "UE4_ROOT is not defined, or points to a non-existant directory, please set this environment variable."
else
  log "Using Unreal Engine at '$UE4_ROOT'"
fi

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Unpack and copy over CarlaUE4's Exported Maps"

USAGE_STRING="Usage: $0 [-h|--help] [-d|--dir] <outdir> [-f|--file] <filename>"

OUTPUT_DIRECTORY=""
FILE_NAME=""

OPTS=`getopt -o h,d::,f --long help,dir::,file:: -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    --dir )
      OUTPUT_DIRECTORY="$2"
      shift ;;
    --file )
      FILE_NAME="$2"
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      break ;;
  esac
done

if [ -z "${FILE_NAME}" ]; then
  FILE_NAME="CookedExportedMaps"
fi

#Tar.gz the stuff
for filepath in `find ExportedMaps/ -type f -name "*.tar.gz"`; do
  tar --keep-newer-files -xvf ExportedMaps/${filepath}.tar.gz
done


#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Unpack and copy over CarlaUE4's Exported Maps"

USAGE_STRING="Usage: $0 [-h|--help] [-d|--dir] <outdir>"

OUTPUT_DIRECTORY=""

OPTS=`getopt -o h,d:: --long help,dir:: -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    --dir )
      OUTPUT_DIRECTORY="$2"
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

#Tar.gz the stuff
for filepath in `find ExportedMaps/ -type f -name "*.tar.gz"`; do
  tar --keep-newer-files -xvf ${filepath}
done


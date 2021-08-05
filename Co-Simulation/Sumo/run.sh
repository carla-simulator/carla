#! /bin/bash

# $1 is scenario name, and the name should be included in ./envs/scenario2envfile.json

sh kill_all_procs.sh > /dev/null 2>&1
python -u run.py $1 | tee ./log/stdout.txt &

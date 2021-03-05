#! /bin/bash

sh kill_all_procs.sh > /dev/null 2>&1
python -u run.py | tee ./log/stdout.txt

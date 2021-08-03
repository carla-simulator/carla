#! /bin/bash

LOCK_FILE="*.lock"
LOCK_FILE=$1

while [ -f ${LOCK_FILE} ]
do
  	echo "ファイルが存在します"
	continue
done

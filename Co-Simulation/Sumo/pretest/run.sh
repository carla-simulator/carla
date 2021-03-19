#! /bin/bash

c=0
while :
do
	python send_sample_data_to_data_server.py
	echo $c
	c=$(($c + 1))
done
	

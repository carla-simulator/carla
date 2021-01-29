#!/bin/bash
netgen -c square/square.netg.cfg
python patchTrafficLights.py -n square/square.tmp.net.xml -o square/square.net.xml
rm square/square.tmp.net.xml

netgen -c kuehne/kuehne.netg.cfg
python patchTrafficLights.py -n kuehne/kuehne.tmp.net.xml -o kuehne/kuehne.net.xml
rm kuehne/kuehne.tmp.net.xml

netconvert -c cross/cross.netc.cfg
echo manually patch traffic lights in cross.net.xml:
echo replace tl-logic with those in cross/cross.tls.add.xml 

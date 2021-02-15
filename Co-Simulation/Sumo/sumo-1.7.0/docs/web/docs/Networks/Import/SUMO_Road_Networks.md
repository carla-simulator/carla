---
title: Networks/Import/SUMO Road Networks
permalink: /Networks/Import/SUMO_Road_Networks/
---

[netconvert](../../netconvert.md) allows a round-trip reimport of
generated SUMO road networks. Use the option **--sumo-net-file** {{DT_FILE}} for reading a SUMO road
network, the shorter options **--sumo-net** {{DT_FILE}} and **--sumo** {{DT_FILE}} are synonymes. An example call:

```
netconvert --sumo-net-file mySUMOnet.net.xml -o mySUMOnet2.net.xml
```

Note that in this case, both the read network ("mySUMOnet.net.xml") and
the generated one ("mySUMOnet2.net.xml") should be same besides the
meta-information in the XML header.

Re-importing SUMO networks allows you to apply changes to an existing
road network using additional
[XML-files](../../Networks/PlainXML.md).
An example would be patching the type of a node.

prepare a file like this: patch.nod.xml :

```
<nodes>
    <node id="id_of_the_node_you_want_to_modify" type="right_before_left"/>
<nodes>
```

and patch the network like this:

```
netconvert --sumo-net-file your.net.xml --node-files patch.nod.xml -o yourpatched.net.xml
```
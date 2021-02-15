---
title: Developer/XML Validation
permalink: /Developer/XML_Validation/
---

# Status Quo (2018-10-12)

- The current build scripts for windows and linux execute
[schemaCheck.py]({{Source}}tools/xml/schemaCheck.py)
as part of the regular tool tests with results visible on the [daily
build page](https://sumo.dlr.de/daily/) (see for instance the schema
check tests on the very end of the [64bit linux tool tests
page](https://sumo.dlr.de/daily/gcc4_64report/tools/test_default.html)).
- This script uses
lxml (if
available) or
[Sax2Count](http://xerces.apache.org/xerces-c/sax2count-3.html) (on
windows only). The latter comes together with the binary xerces
windows distribution.
- Only files which have a schema declaration in the root element and
are of the following type (and fit the following naming scheme) are
checked:
  - netconvert inputs
    - edges "\*.edg.xml"
    - nodes "\*.nod.xml"
    - connections "\*.con.xml"
    - types "\*.typ.xml"
  - sumo nets "\*.net.xml", "net.netgen", "net.netconvert"
  - route files "\*.rou.xml", "routes.duarouter", "alts.duarouter",
    "routes.jtrrouter" including vehroutes output
  - additional files "\*.add.xml"
  - configuration files "\*.????cfg"
  - sumo rawdump (netstate), detector (e1 and e2) and edgedata (net,
    emissions and noise) outputs
- There is the possibility to generate the XML schema for the
configurations (command line option **--save-schema** {{DT_FILE}})
- XML validation can be configured for all binary executables using **--xml-validation**

## portable testing application (only kept for reference)

28.10.2009: I made a small search on possible tools for validating XML
files which are using Python. It seems as two are majorly used:

- [lxml](https://lxml.de/validation.html)
- [XSV](http://www.ltg.ed.ac.uk/~ht/xsv-status.html)

further candidates:

- [xmlproc](http://www.garshol.priv.no/download/software/xmlproc/)
- [minixsv](http://www.familieleuthe.de/MiniXsv.html)

A short comparison:

|                    | lxml                                   | XSV         | xmlproc | minixsv  |
| ------------------ | -------------------------------------- | ----------- | ------- | -------- |
| **Execution Type** | library                                | application | library | library  |
| **Dependencies**   | libxml 2.6.21 or later, libxslt 1.1.15 | ?           | ?       | genxmlif |
---
title: Networks/Import/DlrNavteq
permalink: /Networks/Import/DlrNavteq/
---

The DLR-Navteq importer reads Navteq's GDF databases converted using
Elmar's tools. The importer reads unsplitted network versions. The
option for reading them is **--dlr-navteq-prefix** {{DT_FILE}} or **--dlr-navteq** {{DT_FILE}} for short. Please note that the prefix
must be given only, example:

```
netconvert --dlr-navteq=berlin_ --output-file=MySUMOFile.net.xml
```
---
title: Basics/Notation
permalink: /Basics/Notation/
---

This documentation uses coloring to differ between
different type of information. Below, these annotations and colors are
described.

## Command Line

If you encounter something like this:

```
netconvert --visum=MyVisumNet.inp --output-file=MySUMONet.net.xml
```

you should know that this is a call on the command line. There may be
also a '\\' at the end of a line. This indicates that you have to
continue typing without pressing return (ignoring both the '\\' and the
following newline). The following example means exactly the same as the
one above:

```
netconvert --visum=MyVisumNet.inp \
  --output-file=MySUMONet.net.xml
```

## Application Options

Command line option names are normally colored **this way**. Their values ** *<LIKE THIS\>* **.

## XML Examples

XML-elements and attributes are shown `like this`. Their values, if variable,
*`<LIKE THIS>`*.

Complete examples of XML-Files are shown like the following:

    <myType>
       <myElem myAttr1="0" myAttr2="0.0"/>
       <myElem myAttr1="1" myAttr2="-500.0"/>
    </myType>

## Referenced Data Types

- *<BOOL\>*: a boolean value, use "t" or "true" and "f" or "false" for
  encoding
- *<INT\>*: an integer value, may be negative
- *<UINT\>*: an unsigned integer value, must be \>=0
- *<FLOAT\>*: a floating point number
- *<TIME\>*: time, given in seconds; fractions are allowed, e.g. "12.1"
- *<STRING\>*: any string, but use ASCII-characters only
- *<ID\>*: a string which must not contain the following characters:
  '\#'

  !!! caution
      The list of not allowed characters is incomplete


- *<FILE\>* or *<FILENAME\>*: the (relative or absolute) path to a file;
  see also [\#Referenced File Types](#referenced_file_types)
- *<PATH\>*: a (a relative or absolute) path (usually to a folder)
- *<COLOR\>*: a quadruple of floats separated by ','
  (*<FLOAT\>*,*<FLOAT\>*,*<FLOAT\>*,*<FLOAT\>*), which describe the red,
  green, blue, and alpha component ranging from 0.0 to 1.0 (the alpha
  component is optional), alternatively the list may contain integers
  in the 0-255 range. Please note that the separator must be a comma
  and there are no spaces allowed. The color may also be defined using
  a single string with a [HTML color code](https://en.wikipedia.org/wiki/Web_colors#Hex_triplet) or one of
  the basic colors ("red", "green", "blue", "yellow", "cyan",
  "magenta", "black", "white", "grey")
- *<2D-POSITION\>*: two floats separated by ','
  (*<FLOAT\>*,*<FLOAT\>*), which describe the x- and the y-offset,
  respectively. z is 0 implicitly
- *<3D-POSITION\>*: three floats separated by ','
  (*<FLOAT\>*,*<FLOAT\>*,*<FLOAT\>*), which describe the x- , y- and the
  z-offset, respectively
- *<POSITION-VECTOR\>*: A list of 2D- or 3D-Positions separate by ' '.
  I.e. (*<2D-POSITION\>* *<2D-POSITION\>*,*<3D-POSITION\>*)
- *<2D-BOUNDING_BOX\>*: four floats separated by ','
  (*<FLOAT\>*,*<FLOAT\>*,*<FLOAT\>*,*<FLOAT\>*), which describe x-minimum,
  y-minimum, x-maximum, and y-maximum
- *<PROJ_DEFINITION\>*: a string containing the projection definition
  as used by proj.4; please note that you have to embed the definition
  string in quotes

### Referenced File Types

- *<NETWORK_FILE\>*: a [SUMO network file](../Networks/SUMO_Road_Networks.md) as built by
  [netgenerate](../netgenerate.md) or
  [netconvert](../netconvert.md)
- *<ROUTES_FILE\>*: a [SUMO routes file](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md)
  as built by [duarouter](../duarouter.md) or
  [jtrrouter](../jtrrouter.md) or by hand
- *<TYPE_FILE\>*: a [SUMO edge type file](../SUMO_edge_type_file.md), built by hand or downloaded
- *<OSM_FILE\>*: a [OpenStreetMap file](../OpenStreetMap_file.md)
  as exported from [OpenStreetMap](https://www.openstreetmap.org/)

## Further Schemes

Brackets '\[' and '\]' indicate that the enclosed information is
optional. Brackets '<' and '\>' indicate a variable - insert your own
value in here.

{{SUMO}} is the path you have saved your SUMO-package into.
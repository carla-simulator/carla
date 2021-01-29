---
title: Developer/How To/Net Importer
permalink: /Developer/How_To/Net_Importer/
---

In the following, advices on implementing a new network importer are
given-

Implementing an own network import module is very easy. The following
steps are necessary:

- write methods which read and parse the information from the original
  file
- insert parsed values into the netbuilding containers
- make the network importer know and use your code
- submit your code

Yes, that's all. Still, the three steps are described more detailed in
the following sections.

# Building a Reader

## Initial Considerations

There are some conventions on network importers we try to follow. They
are not mandatory, but they are assumed to assure the code's
extendibility and to force a higher quality of the code.

When starting to write your own importer, you should consider to base it
on an existing one. NIImporter_OpenStreetMap.\*, NIXML...Handler.\* are
examples of reading network descriptions form XML files.
NIImporter_VISUM.\* and NIImporter_DlrNavteq.\* read plain text files,
formatted in different ways. All these modules are located in
{{SUMO}}/src/netimport and of course, your import should be placed here, too.

!!! convention
    network importer code is located in {{SUMO}}/src/netimport/

!!! convention
    network importer should be named NIImporter_<FORMAT_NAME\>

## Calling the Reader

Before adding a call to the init-function of your network importer, you
have to add according options; otherwise the first call will end with an
error due to accessing an unknown option. The options have to be added
to void NIFrame::fillOptions().

Then, you can make the importing facility know your parser by including
the .h-file of your parser to NILoader.cpp and add a call to his
loadNetwork-method within void NILoader::load(OptionsCont &oc).

## Import Order

In most cases, edges, nodes, traffic lights, connections between edges,
and other road network attributes are given explicitly and separated.
The information **must** be imported in the correct order. The following
order is right:

- nodes
- edges (need references to origin and to destination nodes)
- connections between edges (need usually references to edges)
- traffic lights (need references to nodes)

Some more outrageous programmers may try to optimise the order a bit if
the original data is aligned different. This may be possible in some
cases.

In the case nodes, edges, and other information are parsed from several
files, the import order must be considered. Of course, it is also
possible to store data in temporary containers. Still, this should be
avoided as it increases memory consumption, and yes, memory consumption
is one of the major problems for the network building process. In the
case temporary containers are used, they must of course be cleared
before returning from importing.

!!! convention
    try to avoid static containers

!!! convention
    clean static containers if used

## Starting to Parse

All importers have a static method named "loadNetwork" which starts the
parsing. This method is responsible for starting the import if the
according options are set and to assure the read values are stored for
further processing within the according netbuilding containers. For this
purpose, it is necessary to have the options and the netbuilding
structure be given to the importer, making the call have the following
fingerprint:

```
static void loadNetwork(const OptionsCont &oc, NBNetBuilder &nb);
```

At first, this method asks whether the option which makes
[netconvert](../../netconvert.md) read a file of the according format
is set and whether the set value is right. By now, this means only that
a check whether the given file(s) really exist(s) is made, not whether
it is well formatted or valid in any means. The method
FileHelpers::exists(<FILE\>) returns whether a given file is existing.

!!! convention
    all importer have a static member which checks whether the according option is set with a valid value

After the file has been checked for being existent, a parser is built
and started.

You may have noticed, that some of the import parsers allow to give more
than one file, divided by ','. This is of a great help sometimes, still
it is not mandatory. It is up to you to decide whether your importer
should be able to handle several files or not.

## Error Handling

Different formats have different sources for being erroneous. In some
cases, heuristics may have to be applied during the parsing. In all
cases something is strange - and it is not strange each time a network
from the given format is imported - a warning should be generated. This
is done by calling the macro

```
WRITE_WARNING(<TEXT>);
```

If the odd behavior completely disallows further processing of the
imported file, or in the case a read value is not in the right format (a
string is read, though a floating point number was awaited, for
example), a ProcessError should be thrown. The exception must have an
information about the error, for example:

```
throw ProcessError("Number of lanes is <0");
```

Yes, the decision whether a strange value is an error or a warning is
sometimes difficult.

!!! convention
    throw a ProcessError in the case of errors

!!! convention
    warn on strange behavior

!!! convention
    both, a warning and an error exception must have an explanation about the reason supplied

# Adding read Values to the netbuilding Containers

After the definition of a node, an edge, or anything else has been
parsed, the according structure must be built and inserted into the
according netbuilding container.

| Class  | Road element                    |
| ------ | ------------------------------- |
| NBNode | A node (intersection, junction) |
| NBEdge | An edge (road, street)          |
|        |                                 |

The following table shows which definitions are stored into which
containers:

In most cases, these methods return whether the addition was successful.
If it was not, there probably is already an object with the same name.
Normally, the imported networks we have dealt with so far did not
contain duplicates - with the exception of edges which may
bidirectional. In the case the object could not been added, you have to
delete it explicitly. In the case your format is known to contain
duplicates, you should ask whether a duplicate was already built and
only if not, build the instance.

!!! convention
    try to avoid building and deleting of duplicates

# Tests

# Submitting the Code

In the case you do not have a write access to the repository, we would
be very happy if you could issue a pull request or send a patch against
a recent version to [the sumo-dev mailing list](../../Contact.md).
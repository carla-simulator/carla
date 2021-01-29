---
title: Developer/Managing Publications
permalink: /Developer/Managing_Publications/
---

Publications are stored in an extended
[bibtex-file](http://sourceforge.net/apps/trac/sumo/browser/trunk/sumo/docs/sumo.bib)
generated using [JabRef](http://jabref.sourceforge.net/).

# Organising

[JabRef](http://jabref.sourceforge.net/) allows not only to enter your
references, but also to group them. We use the following way to organise
the references:

- **sumo**: All that cite SUMO; in fact, there are some other
references in the bibtex-file, for example in case of reports on
projects where SUMO was used, but the paper itself does not mention
SUMO or papers on theory.
- **projects**: a classification by the project
  - **iTETRIS**: reports on work done in iTETRIS
  - ... further projects ...
- **topic**:
  - **V2X**: vehicular communication papers
  - **TLS**: traffic lights paper
  - ... further topics ...
- **organisation**: a classification by organisation
  - **DLR/TS/VM**: the traffic management group at the Institute of
    Transportation Systems at the German Aerospace Center
  - ... further organisations ...

When adding a new entry, please complete its description, first. Then,
right-click on the entry in the list and assign it to the groups it
belongs to - at least organisation and topics.
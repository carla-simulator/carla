# SUMO Documentation

The SUMO Documentation is generated using [MkDocs](https://www.mkdocs.org/).

You can read the Documentation online at: <https://sumo.dlr.de/docs/>

## Getting started

### Prerequisites
- Python > 3.5 (the *mkdocs-macros-plugin* requires it)

### Installation

Clone this repository (if not already done):
```
git clone https://github.com/eclipse/sumo.git
```

Navigate to this path (`sumo/docs/web`).

Install MkDocs and some MkDocs plugins as well as plantuml:
```
pip install -r requirements.txt
```

To run the built-in development server, use:
```
mkdocs serve
```

`mkdocs serve` re-builds the entire site everytime it detects a change. If you want to only re-build the pages that have been modified, use `mkdocs serve --dirtyreload` ([read more](https://www.mkdocs.org/about/release-notes/#support-for-dirty-builds-990))

**To preview edition you should run the built-in development server and access the Documentation from the localhost address (which by default is 127.0.0.1:8000).**

To generate the final static html files, run:
```
mkdocs build
```
This should create the *site* directory, containing all the necessary HTML and CSS files.

## Contributing

Feel free to contribute and improve the SUMO Documentation. All changes need to be via Pull requests.
You can click on the "Edit on GitHub" button in the top right corner of every page, or just simply press the `e` key on your keyboard.

We encourage you to read [this guide](https://sumo.dlr.de/docs/Editing_Articles.html) on how to edit the articles.

You can also report typos and such, opening an issue [here](https://github.com/eclipse/sumo/issues).
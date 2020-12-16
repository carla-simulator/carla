#!/usr/bin/env bash

carla=/carla
pip3 install -e $carla/Docs/ebook/mkdocs-combine
cd $carla
mkdocs build
mkdocscombine --admonitions-md -o $carla/carla.pd
sed -i 's/---//g' $carla/carla.pd
sed '/TOREMOVE/d' -i $carla/carla.pd
sed 's/&#9989;/Yes/' -i $carla/carla.pd
sed 's/&#10060;/No/' -i $carla/carla.pd
sed '/<div class="build-buttons">/,/<\/div>/d' -i $carla/carla.pd
sed '/<button/d' -i $carla/carla.pd

pandoc --number-sections --toc -f markdown+grid_tables+table_captions -o $carla/carla.pdf $carla/carla.pd --pdf-engine=xelatex \
    --listings -H  $carla/Docs/ebook/listings-setup.tex \
    --template=$carla/Docs/ebook/pandoc-book-template/templates/pdf.latex \
    --toc-depth=2 \
    -V papersize=a4 \
    -V geometry:"top=2cm, bottom=2cm, left=1.5cm, right=1.5cm" \
    -V documentclass="article" \
    -V fontsize=12 \
    -V toc-depth=2

pandoc --toc -f markdown+grid_tables --template $carla/Docs/ebook/pandoc-book-template/templates/epub.html -t epub -o $carla/carla.epub $carla/carla.pd



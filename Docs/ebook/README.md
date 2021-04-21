# Carla documentation as ebook
Convert Carla documentation from mkdocs format to a readable book in different formats (PDF/EPUB/HTML).

### How it works

1- Use modified version of [mkdocscombine](https://github.com/twardoch/mkdocs-combine.git) to combine all *.md files into a single pandoc (book.pd) file with the help of mkdocs.yml. 

2- Use modified version of Pandoc template files from [pandoc-book-template](https://github.com/wikiti/pandoc-book-template.git)

* [pandoc-book-template](https://github.com/wikiti/pandoc-book-template.git) uses chapters/[0-9]-CHAPTERNAME.md format, not using mkdocs.yml file)
* [listings-setup.tex](listings-setup.tex) is taken from this [stackexchange post](https://tex.stackexchange.com/questions/179926/pandoc-markdown-to-pdf-without-cutting-off-code-block-lines-that-are-too-long).
    

3- Use [Pandoc](https://pandoc.org/) to build the .PDF, .EPUB files

### How to use:

```
sudo docker build -f Dockerfile -t carladoc2book:latest .
sudo docker run -v=$PWD/../../:/carla/ carladoc2book:latest  /carla/Docs/ebook/build.sh
```


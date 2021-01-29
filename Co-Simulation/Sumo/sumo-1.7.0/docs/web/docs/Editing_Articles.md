---
title: Help on editing Articles
---

This documentation is written using Markdown syntax. Every page or "article" is a Markdown (.md) file located [here]({{Source}}docs/web/docs) in the official SUMO GitHub repository.
The static html pages are then generated using [Mkdocs](https://www.mkdocs.org/).

Feel free to contribute and improve the SUMO Documentation. All changes need to be via Pull requests. You can click on the "Edit on GitHub" button in the top right corner of every page, or just simply press the `e` key on your keyboard.

!!! note
    Although HTML can be written within Markdown, please avoid using it and keep everything in native Markdown as much as possible.


<br><br>
# **Creating an article**

To creat a new article (or page) just create a Markdown (.md) file in the [documentation directory]({{Source}}docs/web/docs).

!!! note
    Please capitalize the very first letter of the name and use `_` (underscores) instead of leaving a ` ` (space) between words in the filename.

The very first 3 lines of each page should be:

```
---
title: Name of the Article
---
```

<font color="red">If you want to create an fresh new article, please consider first seeing the source code of any existing article, to learn and get some basic notions.</font> 



<br><br>
# **Sections and subsections**

Sections and subsections in a document are marked by using the `#` character.

\# is equivalent to a <h1\> section, \## to an <h2\> and so on.

!!! note
    On the toc (table of content) sidebar you see on each page, only **\#**, **\##** and **\###** sections are listed. Despite that, all other levels generate an internal link and therefore can be referenced and linked from another page.

## Example
```
#### This is a sub section, equivalent to <h4>
```
***will display:***

#### This is a sub section, equivalent to <h4\>




<br><br>
# **Styling text**

Element | Code | Will show as
--------|------|--------
Bold | \*\*test\*\* | **test**
Italic | \*test\* | *test*
Bold + Italic | \*\*\*test\*\*\* | ***test***
Strikethrough | \~\~test\~\~ | ~~test~~

Markdown is compatible with the `<sub>` and `<sup>` html-Tags.

- `η<sub>recup</sub>=e<sup>x</sup>` η<sub>recup</sub>=e<sup>x</sup>

## Example
```
Lorem *ipsum* dolor sit amet, *consetetur sadipscing elitr*, sed diam **nonumy** eirmod **tempor invidunt ut labore** et ~~dolore~~ magna ***aliquyam erat, sed diam*** voluptua.
```
***will display:***

Lorem *ipsum* dolor sit amet, *consetetur sadipscing elitr*, sed diam **nonumy** eirmod **tempor invidunt ut labore** et ~~dolore~~ magna ***aliquyam erat, sed diam*** voluptua.


<br><br>
# **Escape character**

Use the escape character in order to display literal characters which would have special meaning in Markdown. The escape character is `\` (backslash). When writing text inside the `` marks (inline code) or in a code block, no escape character are required.

Here are some expressions that need an escape character:

- \@ : if not, it would be taken as a GitHub username
- \# : if not, it would be taken as a Header or a GitHub issue
- \| : when using it inside a table
- \>
- \{
- \}
- \*

## Example
```
`<TEST1>` using code. No escape character is required here.
<TEST2> not using the escape character. Here the element disappears.
<TEST3\> using the escape character
```
***will display:***

`<TEST1>` using code. No escape character is required here.<br>
<TEST2> not using the escape character. Here the element disappears.<br>
<TEST3\> using the escape character



<br><br>
# **Images**

All images must be saved in the [images directory]({{Source}}docs/web/docs/images).
To insert an image, just use the usual Markdown sytax: 

```
![<alt>](images/Wikicommons_rail_fast.jpg "<title>")
```
replace `<alt>` with the alternate text and `<title>` with the title to show on Mouseover.

!!! note
    The `<alt>` and `<title>` attributes are not necessary required, as you can see in the example.

## Example
```
![](images/Wikicommons_rail_fast.jpg)
```
***will display:***

![](images/Wikicommons_rail_fast.jpg)




<br><br>
# **Tables**

See this [Online Tables generator](https://www.tablesgenerator.com/markdown_tables) to get help creating a Table in Markdown, or to convert an existing spreadsheet (e.g. Microsoft Excel) into Markdown.

A table has a Header (the first row), which is separated by hyphens `-` from the rest of the Table.
To separate columns, just use a pipe `|`.

## Example
```
|   |   |   |   |   |
|---|---|---|---|---|
|   |   |   |   |   |
|   |   |   |   |   |
|   |   |   |   |   |
```
***will display:***

|   |   |   |   |   |
|---|---|---|---|---|
|   |   |   |   |   |
|   |   |   |   |   |
|   |   |   |   |   |

!!! note
    It is important to leave an empty line (line break) immediately before and immediately after a table.



<br><br>
# **Math**

In order to insert Mathematical expressions (symbols or equations), you should write them in [LaTeX format](https://en.wikibooks.org/wiki/LaTeX/Mathematics). Math elements are being displayed as images, using a html `<img>` element.
Just replace `<HERE>` with your math expression in the following html element:

`<img src="http://latex.codecogs.com/gif.latex?<HERE>" border="0" style="margin:0;"/>`


## Example
```
<img src="http://latex.codecogs.com/gif.latex?R" border="0" style="margin:0;"/>
```
***will display:***

<img src="http://latex.codecogs.com/gif.latex?R" border="0" style="margin:0;"/>

<hr>
```
<img src="http://latex.codecogs.com/gif.latex?F(x)=\int^a_b\frac{1}{3}x^3" border="0" style="margin:0;"/>
```
***will display:***

<img src="http://latex.codecogs.com/gif.latex?F(x)=\int^a_b\frac{1}{3}x^3" border="0" style="margin:0;"/>




<br><br>
# **Macros / Variables**

In order to make writing this Documentation easier, we implemented some Macros. Macros replace some special text with other content.

## Example

\{\{SUMO\}\}
\{\{DT_FILE\}\}
\{\{DT_STR\}\}
\{\{Version\}\}

***will display:***

{{SUMO}}
{{DT_FILE}}
{{DT_STR}}
{{Version}}

!!! note
    The full list of available Macros can be found [here](https://github.com/eclipse/sumo/blob/master/docs/web/mkdocs.yml) at the end in the `extra` section.

!!! caution
    Macros do not work inside inline code or code blocks.


<br><br>
# **Links**

When linking to a page within this Documentation, please consider the absolute path to the Markdown file.
<font color="red">Do not forget to add the `.md` extension to every internal link.</font> 

Links to external pages will display an arrow at the end (e.g. [DLR](https://www.dlr.de)).

## **Links to files in the SUMO GitHub repo**

Using the \{\{Source\}\} Macro, linking to files in the official SUMO repository on GitHub is as easy as: 

- \[Read the gitignore file\](\{\{Source\}\}.gitignore) [Read the gitignore file]({{Source}}.gitignore)

!!! note
    The \{\{Source\}\} macro is equivalent to https://github.com/eclipse/sumo/blob/master/ including the `/` at the end. Please analyze the following example:

- \[\{\{SUMO\}\}/src/sumo_main.cpp\](\{\{Source\}\}src/sumo_main.cpp) [{{SUMO}}/src/sumo_main.cpp]({{Source}}src/sumo_main.cpp)

## **Internal Links**

Internal links are used to refer a special section of an article. You can obtain the internal link to a section by hovering the cursor next to the section title (an \# character will appear).

Linking to a section within the same page is as easy as this: [Read about Math](#math) `[Read about Math](#math)`

!!! caution
    All texts in internal links are in lower case.

To reference a section on another page, do not forget to put the absolute path and the `.md` extension, like this: [Read about SUMO input](sumo.md#input) `[Read about SUMO input](sumo.md#input)`

Some characters are being ignored (deleted) or replaced when generating an internal link:

- `.` is ignored
- `:` is ignored
- `?` is ignored
- `'` is ignored
- `"` is ignored
- `/` is ignored
- `(` is converted to `_`
- `)` is converted to `_`
- ` ` (space) is converted to `_`
- `>` is converted to `62`

Here are some practical examples:

| Section title | Actual link  |
|---|---|
| tools fail with a SyntaxError or ImportError or some TypeError concerning ">>"  | [#tools_fail_with_a_syntaxerror_or_importerror_or_some_typeerror_concerning_6262](FAQ.md#tools_fail_with_a_syntaxerror_or_importerror_or_some_typeerror_concerning_6262)  |
| Why do I get errors about missing files / file not found even though the file exists?  | [#why_do_i_get_errors_about_missing_files_file_not_found_even_though_the_file_exists](FAQ.md#why_do_i_get_errors_about_missing_files_file_not_found_even_though_the_file_exists)  |
| Version 1.3.1 (27.08.2019)  | [#version_131_27082019](ChangeLog.md#version_131_27082019)  |

!!! note
    No permalink ends with a `_` character.

## Example
```
Go to [sumo-gui](sumo-gui.md), [netconvert options](netconvert.md#options) or [Notation](Basics/Notation.md)

<https://www.dlr.de> or [DLR](https://www.dlr.de)
```
- \[Read the gitignore file\](\{\{Source\}\}.gitignore) 
- \[\{\{SUMO\}\}/src/sumo_main.cpp\](\{\{Source\}\}src/sumo_main.cpp)

***will display:***

Go to [sumo-gui](sumo-gui.md), [netconvert options](netconvert.md#options) or [Notation](Basics/Notation.md)

<https://www.dlr.de> or [DLR](https://www.dlr.de)

- [Read the gitignore file]({{Source}}.gitignore)
- [{{SUMO}}/src/sumo_main.cpp]({{Source}}src/sumo_main.cpp)

<br><br>
# **GitHub issues, commits and users**

In order to use the \# and \@ characters freely, please use the [escape character](#escape_character).

Element | Code | Will show as
--------|------|--------
GitHub issue | \#5697 | #5697
GitHub user | \@namdre | @namdre
GitHub commit | `3aa106d1bf3221031cca75cfe259913d9d3e88ae` (the complete Commit Hash) | 3aa106d1bf3221031cca75cfe259913d9d3e88ae

## Example
```
The issue #5697 was reported by @angelobanse. @behrisch commited 3aa106d1bf3221031cca75cfe259913d9d3e88ae to close that issue.
```
***will display:***

The issue #5697 was reported by @angelobanse. @behrisch commited 3aa106d1bf3221031cca75cfe259913d9d3e88ae to close that issue.



<br><br>
# **Notes**

The ***NOTE*** and ***CAUTION*** elements are useful for highlighting important information.
You can also use a custom note type.

Inside a **Note** element you can use Markdown syntax to insert links, stylize text and even insert images.

!!! caution
    Unfortunately, you cannot insert a note in a Table.

## Example
```
!!! note
    When citing SUMO in general please use our current reference 
    publication: ["Microscopic Traffic Simulation using SUMO"](https://elib.dlr.de/124092/); 
    Pablo Alvarez Lopez, Michael Behrisch, Laura Bieker-Walz, Jakob Erdmann, Yun-Pang Flötteröd, 
    Robert Hilbrich, Leonhard Lücken, Johannes Rummel, Peter Wagner, and Evamarie Wießner. 
    IEEE Intelligent Transportation Systems Conference (ITSC) 2018.

!!! caution
    The list of not allowed characters is incomplete

!!! incomplete "Missing:"
    This is just a test
```
\{\{Outdated\}\}


***will display:***

!!! note
    When citing SUMO in general please use our current reference publication: ["Microscopic Traffic Simulation using SUMO"](https://elib.dlr.de/124092/); Pablo Alvarez Lopez, Michael Behrisch, Laura Bieker-Walz, Jakob Erdmann, Yun-Pang Flötteröd, Robert Hilbrich, Leonhard Lücken, Johannes Rummel, Peter Wagner, and Evamarie Wießner. IEEE Intelligent Transportation Systems Conference (ITSC) 2018.

!!! caution
    The list of not allowed characters is incomplete

!!! incomplete "Missing:"
    This is just a test

{{Outdated}}



<br><br>
# **See also**

- [Markdown Cheatsheet](https://guides.github.com/pdfs/markdown-cheatsheet-online.pdf)
- [GitHub Guide on Mastering Markdown](https://guides.github.com/features/mastering-markdown/)
- [SUMO Documentation images]({{Source}}docs/web/docs/images)
- [Online Markdown Tables generator](https://www.tablesgenerator.com/markdown_tables)
- [LaTeX Mathematics](https://en.wikibooks.org/wiki/LaTeX/Mathematics)

- [How to Build this Documentation]({{Source}}docs/web/README.md)
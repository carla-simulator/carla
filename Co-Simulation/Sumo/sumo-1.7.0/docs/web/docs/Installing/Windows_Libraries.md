---
title: Installing/Windows Libraries
permalink: /Installing/Windows_Libraries/
---

!!! note
    Most of the time it is not necessary to build the libraries yourself. You can just use the versions available at <https://github.com/DLR-TS/SUMOLibraries> as described in the [Windows build instructions](../Installing/Windows_Build.md#libraries).

## Individual packages

### Xerces-C

SUMO uses Xerces-C 3.1.x or 3.2.x by default but if you are still stuck
with Xerces 2.8 changes should be limited to [{{SUMO}}/src/windows_config.h]({{Source}}src/windows_config.h), [{{SUMO}}/build/msvc10/Win32.props]({{Source}}build/msvc10/Win32.props) and / or [{{SUMO}}/build/msvc10/x64.props]({{Source}}build/msvc10/x64.props). Please be
aware that Xerces-C 3.1.4 can cause problems when using unicode
characters in your networks (especially importing chinese street names
from OpenStreetMap).

- Apache does not provide binary distributions any longer, but you can
  try our own build of Xerces-C 3.2.0 with an additional patch fixing
  unicode problems [in a 32 bit
  version](http://sourceforge.net/projects/sumo/files/dependencies/xerces-c-3.2.0-win32vc12.zip)
  or its [64bit
  version](http://sourceforge.net/projects/sumo/files/dependencies/xerces-c-3.2.0-win64vc12.zip)
  for the 64bit build.
- Create a environment variable XERCES pointing to your xerces-c
  directory, e.g.

```
set XERCES=D:\libs\xerces-c-3.2.0-win32
```

- Copy dll-files `%XERCES%\bin\xerces-c_3_2.dll` and
  `%XERCES%\bin\xerces-c_3_2D.dll` to some directory which is in your
  PATH (last resort: into Windows' system32 directory).
- For the 64bit build the name of the variable is XERCES_64

### Fox

- If you do not need a GUI, you can skip this section.
- Usually you do not need to build Fox yourself you can [download the
readily compiled
binaries](http://sourceforge.net/projects/sumo/files/dependencies/).
- If you insist, please [download fresh
sources](http://www.fox-toolkit.org/download.html) and do not use
the incomplete package.
- Up to (and including) SUMO 0.10.3 (precisely up to svn revision
7025) SUMO used Fox 1.4, now it uses Fox 1.6. The building
instructions are the same just replace 1.6 by 1.4 (and FOX16 by
FOX14) wherever you need it.
- Go to the Fox directory and open the VC project e.g.
D:\\libs\\fox-1.6.36\\windows\\vcpp\\win32.dsw.
  - If you are using the Visual Studio 10.0 *Express* version, it
    cannot convert the project directly, you will need an
    intermediate Visual Studio 8.0 conversion
- Confirm the conversion to VC 10.0 and build the foxdll project as
"release and debug" (if you think you might wish to use the Visual
Studio debugger) version. If you want to build for the 64bit
platform you need to add a new configuration to the foxdll project
using the
[Configuration Manager](http://msdn.microsoft.com/en-us/library/t1hy4dhz(VS.80>).aspx).
- You might get approximately 240 warnings and one error, which can
probably be ignored as long as you can find the resulting DLLs in
the lib dir.
- Errors on not finding windows.h mean the SDK was not installed
properly.
- Create an environment variable FOX16 pointing to your Fox directory,
e.g.

```
set FOX16=D:\libs\fox-1.6.36.
```

- Copy dll-files `%FOX16%\libs\fox-1.6.36\lib\FOXDLL-1.6.dll` and
  `%FOX16%\lib\FOXDLLD-1.6.dll` to some directory which is in your
  PATH (last resort: into Windows' system32 directory).

#### PNG support in Fox

- Warning: This is a little advanced, we were successful with libpng
1.5.2 and 1.6.20 together with zlib 1.2.5 and 1.2.8
- download zlib sources and libpng sources from
[\[1\]](http://ftp.fox-toolkit.org/pub/) or from
[\[2\]](http://www.libpng.org/pub/png/libpng.html) and
[\[3\]](http://www.zlib.net/)
- unzip all zips
- there is no need to build zlib
- build pnglib by first adapting the path in
<libpng\>\\projects\\vstudio\\zlib.props (if necessary) and then use
the vstudio.sln from the same dir to build the libpng project
- you will get a png lib and a dll in
<libpng\>\\projects\\vstudio\\Debug\\
- if you want to be really sure, you can build the pngtest project as
well and check the output
- you can build / add futher configurations / platforms if needed
- Open the Fox project solution and edit properties of foxdll
  - in the C++/General section add the <libpng\> dir to the include
    path
  - in the C++/Preprocessor section add the HAVE_PNG_H define
  - in the Linker/Input section add the .lib file to the input
- recompile fox and put the resulting dll together with libpng dll in
your path

#### JPEG support in Fox

- Warning: This is a little advanced, we were successful with jpeg 8d,
and 9b the jpeg 8c which can be found at the fox-toolkit site was
broken
- download libjpeg sources from [\[4\]](http://www.ijg.org/) and unzip it
- download
[win32.mak](http://www.bvbcode.com/code/f2kivdrh-395674-down) and
place it in the jpeg dir
- open a visual studio command prompt, cd to the jpeg dir and execute

```
nmake /f makefile.vc setup-v10
nmake /f makefile.vc
```

- you will get a libjpeg lib (and no dll\!) in the jpeg dir
- if you need different configurations / platforms you can also open
the solution file jpeg.sln and build jpeg from the GUI

<!-- end list -->

- Open the Fox project solution and edit properties of foxdll
  - in the C++/General section add the jpeg dir to the include path
  - in the C++/Preprocessor section add the HAVE_JPEG_H define
  - in the Linker/Input section add the .lib file to the input
- recompile foxdll (see above) and put the resulting dll (which has
jpeg support linked statically inside) in your path

### PROJ and GDAL

If you do not need transformation of geocoordinates you can disable PROJ
and GDAL in
[build/msvc10/Win32.props](http://apps.sourceforge.net/trac/sumo/browser/trunk/sumo/build/msvc10/Win32.props)
and / or
[build/msvc10/x64.props](http://apps.sourceforge.net/trac/sumo/browser/trunk/sumo/build/msvc10/x64.props)
(remove the UserMacros lines referencing the relevant \*.lib files and
remove the Preprocessor definitions HAVE_PROJ and/or HAVE_GDAL) and
skip this section. Otherwise you have the choice between downloading the
binary packages by Tamas Szekeres (preferred choice), the FWTools
distribution (a little outdated) or compiling for yourself (for the
adventurous). Building from source should only be necessary if you want
a 64bit build or if you want to ship a copy of the SUMO executable(s)
without the need to ship the FWTools as well.

#### Tamas Szekeres binary packages at <http://www.gisinternals.com/release.php>

- Choose the latest stable release and then the [binaries
  zip](http://download.gisinternals.com/sdk/downloads/release-1600-gdal-1-11-1-mapserver-6-4-1.zip)
  and the [libs
  zip](http://download.gisinternals.com/sdk/downloads/release-1600-gdal-1-11-1-mapserver-6-4-1-libs.zip)
  (links valid for MSVC10 as of 31/8/2015)
- unzip the zips in a directory of your choice (lets say
  D:\\libs\\proj_gdal) and set PROJ_GDAL=D:\\libs\\proj_gdal.
- Add `%PROJ_GDAL%\bin` to your PATH (or copy the contents to some
  directory in your PATH).

#### FWTools (very outdated)

- Just execute the binary and select a target directory.
- Create a environment variable PROJ_GDAL pointing to that directory,
  e.g.

```
set PROJ_GDAL=D:\libs\FWTools1.1.3.
```

- Add `%PROJ_GDAL%\bin` to your PATH (or copy the contents to some
  directory in your PATH).

#### Building from source

If you want to build PROJ and GDAL yourself for some reason, please
follow the relevant [build instructions for
PROJ](http://trac.osgeo.org/proj/browser/trunk/proj/README) and [for
GDAL](http://trac.osgeo.org/gdal/wiki/BuildingOnWindows). You then
should install in a common directory and let the PROJ_GDAL variable
point to it as above. You should also copy proj.dll and gdal\*.dll to a
directory in your PATH.

If you do or download a 64bit build, the name of the environment
variable needs to be PROJ_GDAL_64.

### FFMPEG

This is only needed, if you want to save videos directly from the
sumo-gui.

- You just need to install [the binary ffmpeg
  distributions](http://ffmpeg.zeranoe.com/builds/) in the Shared and
  the Dev flavor. Also be sure to download all the platforms (32
  and/or 64 bit) you need.
- Unzip the Dev version and create a environment variable FFMPEG
  pointing to the directory, e.g.

```
set FFMPEG=D:\libs\ffmpeg-2.8.3-win32-dev.
```

- Unzip the Share version and copy the dll-files from the bin
  directory to some directory which is in your PATH (last resort: into
  Windows' system32 directory).
- For the 64bit build the name of the variable is FFMPEG_64, e.g.

```
set FFMPEG_64=D:\libs\ffmpeg-2.8.3-win64-dev.
```

- Probably due to a bug in Visual Studio 2010 the resulting sumo-gui
  works only if debugging symbols are generated (which is the default
  for the Debug and the Fastbuild but not for the Release)\! If your
  executable does not start or complains about missing symbols, double
  check whether it is enabled in the linker settings (currently it is
  for the 64 bit internal release).

### OpenSceneGraph

This is only relevant for the 3D GUI. You will need to build at least
OSG 3.4.0 because earlier versions do not work with Visual Studio 2012.
Unfortunately there are no binaries readily available. If you want to
use models which are not in <SUMO\>/data/3D, you can add directories
containing them to the OSG_FILE_PATH environment variable. the build
will look for libraries and includes in the directories below $OSG and
$OSG_64.
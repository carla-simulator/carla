Image Converter
===============

Converts output images of depth and semantic segmentation to a prettier format.

Requires boost_system, boost_filesystem, boost_program_options, libpng, libtiff,
libjpeg and libomp.

Compile with `clang++ -std=c++14 -fopenmp`, for the default compilation just run
make

    make
    ./bin/image_converter -h

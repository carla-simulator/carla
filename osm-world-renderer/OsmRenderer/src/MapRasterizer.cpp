#include "MapRasterizer.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "lunasvg.h"
//#include "stb_image_write.h"

#include <iostream>
#include <string>

using namespace lunasvg;

void MapRasterizer::RasterizeSVG(std::uint8_t* OutMap, std::string SvgString, int Size)
{
    auto SvgDocument = Document::loadFromData(SvgString);

    if(!SvgDocument)
    {
        std::cerr << "ERROR: Rasterizer could not load Svg data." << std::endl;
        return;
    }

    Bitmap RasterizedBitmap = SvgDocument->renderToBitmap(Size, Size, BackgroundColor);
    if(!RasterizedBitmap.valid())
    {
        std::cerr << "ERROR: No valid Bitmap" << std::endl;
        return;
    }
    else
    {
        std::cout << "Bitmap VALID" << std::endl; 
    }

    RasterizedBitmap.convertToRGBA();
    //std::cout << std::hex << RasterizedBitmap.data() << std::endl;
    OutMap = RasterizedBitmap.data();
    
    //std::string basename = "/home/aollero/Downloads/libosmcout/serverOutTest/madrid.png"; 
    //stbi_write_png(basename.c_str(), Size, Size, 4, RasterizedBitmap.data(), 0);
}
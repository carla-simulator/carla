#include "MapRasterizer.h"
#include "OsmRendererMacros.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "lunasvg.h"
//#include "stb_image_write.h"

#include <iostream>
#include <string>
#include <cstring>

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
        std::cerr << LOG_PRFX << "ERROR: No valid Bitmap" << std::endl;
        return;
    }
    else
    {
        std::cout << LOG_PRFX << "Bitmap VALID" << std::endl; 
    }

    RasterizedBitmap.convertToRGBA();
    std::memcpy(OutMap, RasterizedBitmap.data(), Size*Size*4*sizeof(uint8_t));
}
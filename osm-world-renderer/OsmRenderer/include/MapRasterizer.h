#ifndef MAP_RASTERIZER_H
#define MAP_RASTERIZER_H

#include <string>

class MapRasterizer
{
public:
    void RasterizeSVG(std::uint8_t* OutMap, std::string SvgString, int Size);

private:
    int BackgroundColor = 0x00000000;
};

#endif
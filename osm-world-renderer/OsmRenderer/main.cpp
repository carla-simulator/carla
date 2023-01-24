#include <iostream>

#include "include/OsmRenderer.h"

int main()
{
    std::cout << "🚀 Starting renderer" << std::endl;
    OsmRenderer Renderer;
    try
    {
        Renderer.InitRenderer();
        Renderer.StartLoop();
    }
    catch(std::exception& e)
    {
        std::cerr << "ERROR:: " << e.what() << std::endl;
        return -1;
    }

    Renderer.ShutDown();
    std::cout << "Shutting down renderer. Bye!👋 " << Renderer.GetOsmRendererString() << std::endl;

    return 0;
}
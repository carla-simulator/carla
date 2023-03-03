#include <iostream>

#include "include/OsmRenderer.h"

int main()
{
    //std::locale::global(std::locale("C.UTF-8"));
    //std::locale::global(std::locale("es_ES.utf8"));
    // std::locale::global(std::locale("en_US.utf8"));

    std::cout << " Starting renderer " << std::locale().name() << std::endl;
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
    std::cout << "Shutting down renderer. Bye! " << Renderer.GetOsmRendererString() << std::endl;

    return 0;
}
#include <map>
#include <cmath>
#include <shared_mutex>
#include "carla/geom/Location.h"
#include "carla/client/Actor.h"

class CreateGrid
{
private:
    std::map <std::pair < int, int > , std::map< carla::SharedPtr <carla::client::Actor > , int>  > GridIDToActor;
    std::map < carla::SharedPtr <carla::client::Actor >, std::pair < int, int > > ActorToGridID;
    std::pair < int, int> GridID;

public:
    CreateGrid(/* args */);
    ~CreateGrid();
    std::pair < int, int > GetGridID(carla::SharedPtr <carla::client::Actor >);
    std::map< carla::SharedPtr <carla::client::Actor > , int> GetActor(std::pair < int, int >);
};
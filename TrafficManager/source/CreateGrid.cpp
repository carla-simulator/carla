#include "CreateGrid.h"

CreateGrid::CreateGrid(){}
CreateGrid::~CreateGrid(){}

std::shared_timed_mutex mutex_lock;

std::pair < int, int > CreateGrid::GetGridID(carla::SharedPtr <carla::client::Actor > actor)
{
    std::lock_guard<std::shared_timed_mutex> writerLock(mutex_lock);
    GridID.first = std::floor((actor->GetLocation().x)/10);
    GridID.second = std::floor((actor->GetLocation().y)/10);
    //when ActorID is not found in the map (it's a new actor)
    if (ActorToGridID.count(actor) == 0)
    {
        ActorToGridID.insert({ actor, GridID }) ;
        //update in GridIDTOActor map
        if(GridIDToActor.count(GridID) == 0)
        {
            std::map < carla::SharedPtr <carla::client::Actor > , int> Actor_map;
            Actor_map.insert({actor, 1});
            GridIDToActor.insert ({ GridID , Actor_map});
        }
        else
        {
            GridIDToActor.find(GridID)->second.insert({actor, 1});
        }

    }
    //ActorID is found in map but location changed
    else if(ActorToGridID.find(actor)->second != GridID)
    {
        GridIDToActor.find(ActorToGridID.find(actor)->second)->second.erase(actor);
        //Delete if Actor_map is empty
        if (GridIDToActor.find(ActorToGridID.find(actor)->second)->second.size() == 0)
        {
            GridIDToActor.erase(ActorToGridID.find(actor)->second);
        }
        ActorToGridID.erase(actor);
        ActorToGridID.insert ({actor,GridID }) ;

        //update in GridIDTOActor map
        if(GridIDToActor.count(GridID) == 0)
        {
            std::map< carla::SharedPtr <carla::client::Actor > , int> Actor_map;
            Actor_map.insert({actor, 1});
            GridIDToActor.insert ({ GridID , Actor_map});
        }
        else
        {
            GridIDToActor.find(GridID)->second.insert({actor, 1});
        }

    }
    return GridID;

}
std::map< carla::SharedPtr <carla::client::Actor > , int>  CreateGrid::GetActor(std::pair < int, int > GridID)
{
    std::shared_lock<std::shared_timed_mutex> readerLock(mutex_lock);
    if ( GridIDToActor.count(GridID) == 0 )
    {
        std::map< carla::SharedPtr<carla::client::Actor>,int> Actor_map;
        return Actor_map;
    }
    else
    {
        return GridIDToActor.find(GridID)->second;
    }
}
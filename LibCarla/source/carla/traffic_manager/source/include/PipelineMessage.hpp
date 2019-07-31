
#pragma once

#include <string>
#include <map>

#include "carla/client/Actor.h"


namespace traffic_manager {

    class PipelineMessage {
        /*
        This class is used for messaging through the pipeline.
        The class holds information about the vehicle the message is constructed for.
        Also, the class has accessor methods to set and get attributes.
        These attributes can be used to pass data from stage to stage.
        */

        private:

        int actor_id;
        carla::SharedPtr<carla::client::Actor> actor;
        std::map<std::string, float> attributes;

        public:

        PipelineMessage();
        virtual ~PipelineMessage();

        /* This method sets the actor information into the class object. */
        void setActor(carla::SharedPtr<carla::client::Actor> actor);

        /* Returns shared pointer to actor for which the message was constructed. */
        carla::SharedPtr<carla::client::Actor> getActor();

        /* Returns vehicle's actor ID */
        int getActorID();

        /* Sets a float value corresponding to a string key. */
        void setAttribute(std::string, float);

        /* Returns value for the string key passed. */
        float getAttribute(std::string);

        /* Checks if a given attribute is set for the object. */
        bool hasAttribute(std::string);

        /* Removes the attribute with the string key passed. */
        void removeAttribute(std::string);
    };
}

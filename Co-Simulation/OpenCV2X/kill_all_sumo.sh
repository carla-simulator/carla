#! /bin/bash

ps awx | grep sumo | awk '{print $1}' | xargs kill -9



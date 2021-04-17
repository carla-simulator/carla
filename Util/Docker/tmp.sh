#! /bin/bash
docker build -t carla-prerequisites -f Prerequisites.Dockerfile .
docker build -t carla -f Carla.Dockerfile .

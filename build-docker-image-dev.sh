#!/bin/sh
PROJECT_NAME=up_area_creator
DOCKER_TAG="latest"

docker build --no-cache -t $PROJECT_NAME:$DOCKER_TAG -f Dockerfile.dev .
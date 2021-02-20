#!/usr/bin/bash

docker build --tag rack-build:latest -f docker/Dockerfile . && docker run rack-build:latest

#!/bin/sh
echo "STABLE_GIT_TAG $(git describe --tags --dirty --always)"

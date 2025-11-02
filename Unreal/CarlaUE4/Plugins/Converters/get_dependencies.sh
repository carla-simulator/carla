#!/bin/bash

# Remove the ThirdParty folder where all of the links are created
echo Removing existing SimReady Plugin ThirdParty dependencies
rm -rf "SimReady/ThirdParty"
rm -rf "MDL/Source/ThirdParty"
rm -rf MDL/Library/mdl

echo Pulling SimReady dependencies
"SimReady/packman/packman" pull "SimReady/Dependencies/packman.xml" --platform linux-x86_64

echo Pulling MDL dependencies
"SimReady/packman/packman" pull "MDL/Dependencies/packman.xml" --platform linux-x86_64

echo Installing and patching Engine Material source
MDL/Dependencies/Engine/InstallPrivateMaterialInterface.sh

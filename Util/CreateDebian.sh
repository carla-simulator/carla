#!/bin/bash

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# This script builds a debian package for CARLA.
#
# Usage:
#     $ ./CreateDebian.sh <CARLA-VERSION>
#
# Tested with Ubuntu 14.04, 16.04, 18.04 and 19.10.
#
# IMPORTANT: Add/remove the appropriate folders in Makefile at line 81.

# ==================================================================================================
# -- Variables -------------------------------------------------------------------------------------
# ==================================================================================================

if [[ -z $1 ]];
then
  echo "$(date) - Missing mandatory arguments: CARLA version. "
  echo "$(date) - Usage: ./CreateDebian.sh [version]. "
  exit 1
fi

CARLA_VERSION=$1
CARLA_DIR=carla-simulator-${CARLA_VERSION}
CARLA_RELEASE_URL=https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/CARLA_${CARLA_VERSION}.tar.gz
ADDITIONAL_MAPS_URL=https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/AdditionalMaps_${CARLA_VERSION}.tar.gz

# Adding maintainer name.
DEBFULLNAME=Carla\ Simulator\ Team
export DEBFULLNAME

# ==================================================================================================
# -- Dependencies ----------------------------------------------------------------------------------
# ==================================================================================================
# Installing required dependencies.
sudo apt-get install build-essential dh-make

# ==================================================================================================
# -- Download --------------------------------------------------------------------------------------
# ==================================================================================================
mkdir -p carla-debian/"${CARLA_DIR}"
cd carla-debian/"${CARLA_DIR}"

FILE=$(pwd)/ImportAssets.sh
if [ -f "$FILE" ]; then
  echo "Package already downloaded!"
else
  curl "${CARLA_RELEASE_URL}" | tar xz

  wget "${ADDITIONAL_MAPS_URL}"
  mv AdditionalMaps_"${CARLA_VERSION}".tar.gz Import/
fi

# Importing new maps.
./ImportAssets.sh

# Removing unnecessary files
rm CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping.debug
rm CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping.sym

# ==================================================================================================
# -- Debian package --------------------------------------------------------------------------------
# ==================================================================================================
# Updating CarlaUE4.sh script
rm CarlaUE4.sh
cat >> CarlaUE4.sh <<EOF
#!/bin/sh
"/opt/carla-simulator/CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping" CarlaUE4 \$@
EOF

# Making debian package to install Carla in /opt folder
rm Makefile

cat >> Makefile <<EOF
binary:
	# we are not going to build anything

install:
	mkdir -p \$(DESTDIR)/opt/carla-simulator/bin
	cp CarlaUE4.sh \$(DESTDIR)/opt/carla-simulator/bin
	cp ImportAssets.sh \$(DESTDIR)/opt/carla-simulator
	cp -r CarlaUE4 \$(DESTDIR)/opt/carla-simulator
	cp -r Engine \$(DESTDIR)/opt/carla-simulator
	cp -r Import \$(DESTDIR)/opt/carla-simulator
	cp -r PythonAPI \$(DESTDIR)/opt/carla-simulator
	cp -r Co-Simulation \$(DESTDIR)/opt/carla-simulator
	cp -r Tools \$(DESTDIR)/opt/carla-simulator
EOF

# Create necessary file structure for debian packaging
timeout --signal=SIGINT 10 dh_make -e carla.simulator@gmail.com --indep --createorig -y

cd debian/

# Removing unnecessary files
rm ./*.ex
rm ./*.EX

# Adding package dependencies(Package will install them itself) and description
rm control

cat >> control <<EOF
Source: carla-simulator
Section: simulator
Priority: optional
Maintainer: Carla Simulator Team <carla.simulator@gmail.com>
Build-Depends: debhelper (>= 9)
Standards-Version: ${CARLA_VERSION}
Homepage: http://carla.org/

Package: carla-simulator
Architecture: any
Depends: python,
  python-numpy,
  python-pygame,
  libpng16-16,
  libjpeg8,
  libtiff5
Description: Open-source simulator for autonomous driving research
 CARLA has been developed from the ground up to support development, training, and validation
 of autonomous driving systems. In addition to open-source code and protocols, CARLA provides
 open digital assets (urban layouts, buildings, vehicles) that were created for this purpose
 and can be used freely. The simulation platform supports flexible specification of sensor suites,
 environmental conditions, full control of all static and dynamic actors, maps generation and much more.
EOF

# Adding Carla library path (carla.pth) to site-packages, during post installation.
rm postinst

cat>> postinst << EOF
#!/bin/sh

SITEDIR=\$(python3 -c 'import site; site._script()' --user-site)
mkdir -p "\$SITEDIR"
PYTHON3_EGG=\$(ls /opt/carla-simulator/PythonAPI/carla/dist | grep py3.)
echo "/opt/carla-simulator/PythonAPI/carla/dist/\$PYTHON3_EGG\n/opt/carla-simulator/PythonAPI/carla/" > "\$SITEDIR/carla.pth"

SITEDIR=\$(python2 -c 'import site; site._script()' --user-site)
mkdir -p "\$SITEDIR"
PYTHON2_EGG=\$(ls /opt/carla-simulator/PythonAPI/carla/dist | grep py2.)
echo "/opt/carla-simulator/PythonAPI/carla/dist/\$PYTHON2_EGG\n/opt/carla-simulator/PythonAPI/carla/" > "\$SITEDIR/carla.pth"

chmod +x /opt/carla-simulator/bin/CarlaUE4.sh

set -e

case "\$1" in
    configure)
    ;;

    abort-upgrade|abort-remove|abort-deconfigure)
    ;;

    *)
        echo "postinst called with unknown argument \\\`\$1'" >&2
        exit 1
    ;;
esac

exit 0
EOF

# Removing Carla library from site-packages
rm prerm
cat>> prerm << EOF
#!/bin/sh

SITEDIR=\$(python3 -c 'import site; site._script()' --user-site)
rm "\$SITEDIR/carla.pth"

SITEDIR=\$(python2 -c 'import site; site._script()' --user-site)
rm "\$SITEDIR/carla.pth"

set -e

case "\$1" in
    remove|upgrade|deconfigure)
    ;;

    failed-upgrade)
    ;;

    *)
        echo "prerm called with unknown argument \\\`\$1'" >&2
        exit 1
    ;;
esac

exit 0
EOF

# Updating copyright.
rm copyright
cp ../LICENSE ./copyright

# Updating debian/Changelog
awk '{sub(/UNRELEASED/,"stable")}1' changelog > tmp && mv tmp changelog
awk '{sub(/unstable/,"stable")}1' changelog > tmp && mv tmp changelog
cd ..

# Building debian package.
dpkg-buildpackage -uc -us -b


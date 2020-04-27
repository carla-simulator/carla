#!/bin/sh

#This script builds debian package for CARLA,
#Please read comments before making any changes
#Tested with Ubuntu 14.04, 16.04, 18.04 and 19.10

sudo apt-get install build-essential dh-make

#Adding maintainer name
DEBFULLNAME=Carla\ Simulator\ Team
export DEBFULLNAME

#replace CARLA_VERSION with your required carla-<version>
#Note :Please check urls either at lines 28&29 or lines 32&33 works with given CARLA_VERSION
#otherwise add an appropriate if statement with correct links after versionCompare() function.
#For other Carla releases, please add/remove appropriate folder in Makefile at line 80.
CARLA_VERSION=0.9.9
CARLA_VER=9.9 #for carla version like 1.0.1 use 10.1
CARLA_DIR=carla-simulator-${CARLA_VERSION}

#to find correct github link
versionCompare() {
   awk -v v1="$1" -v v2="$2" 'BEGIN {printf v1<v2? 0:1 }'
}

if [ $(versionCompare ${CARLA_VER} 9.8) -eq 1 ]
then
	CARLA_RELEASE_REPO=https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/CARLA_${CARLA_VERSION}.tar.gz
	ADDITIONALMAPS=https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/AdditionalMaps_${CARLA_VERSION}.tar.gz
elif [ $(versionCompare ${CARLA_VER} 9.5) -eq 1 ] && [ $(versionCompare 9.8 ${CARLA_VER}) -eq 1 ]
then
	CARLA_RELEASE_REPO=http://carla-assets-internal.s3.amazonaws.com/Releases/Linux/CARLA_${CARLA_VERSION}.tar.gz
	ADDITIONALMAPS=http://carla-assets-internal.s3.amazonaws.com/Releases/Linux/AdditionalMaps_${CARLA_VERSION}.tar.gz
else
	echo "For put correct link at line 35&36 in CreateDebian.sh for other Carla releases"
	CARLA_RELEASE_REPO=
	ADDITIONALMAPS=
fi

ADDITIONALMAPS_TAR=AdditionalMaps_${CARLA_VERSION}.tar.gz

mkdir -p carla-debian/${CARLA_DIR}
cd carla-debian/${CARLA_DIR}

#Check if Carla-<version> release is already downloaded
FILE=$(pwd)/ImportAssets.sh
if [ -f "$FILE" ]; then
	ImportAssetscheck=1
else
	ImportAssetscheck=0
fi

#If carla release is not already downloaded then it will download
if [ ${ImportAssetscheck} == 0 ]
then
	curl ${CARLA_RELEASE_REPO} | tar xz

	wget ${ADDITIONALMAPS}
	mv ${ADDITIONALMAPS_TAR} Import/

fi

./ImportAssets.sh #importing new maps

#removing unnecessary files
rm CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping.debug
rm CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping.sym

rm CarlaUE4.sh
cat >> CarlaUE4.sh <<EOF
#!/bin/sh
sudo chmod +x "/opt/carla-simulator/CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping"
"/opt/carla-simulator/CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping" CarlaUE4 $@
EOF


rm Makefile

#Making debian package to install Carla in /opt folder
cat >> Makefile <<EOF
binary:
	# we are not going to build anything

install:
	mkdir -p \$(DESTDIR)/opt/carla-simulator/bin
	cp  CarlaUE4.sh \$(DESTDIR)/opt/carla-simulator/bin
	cp ImportAssets.sh \$(DESTDIR)/opt/carla-simulator
	cp -r CarlaUE4 \$(DESTDIR)/opt/carla-simulator
	cp -r Engine \$(DESTDIR)/opt/carla-simulator
	cp -r Import \$(DESTDIR)/opt/carla-simulator
	cp -r PythonAPI \$(DESTDIR)/opt/carla-simulator
	cp -r Co-Simulation \$(DESTDIR)/opt/carla-simulator
	cp -r Tools \$(DESTDIR)/opt/carla-simulator
EOF

timeout --signal=SIGINT 10 dh_make -e carla.simulator@gmail.com --indep --createorig -y  #to create necessary file structure for debian packaging

cd debian/

#removing unnecessary files
rm ./*.ex
rm ./*.EX

rm control

#Adding package dependencies(Package will install them itself) and description
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


rm postinst

#Adding Carla library path (carla.pth) to site-packages, during post installation.
cat>> postinst << EOF
#!/bin/sh

SITEDIR=\$(python3 -c 'import site; site._script()' --user-site)
mkdir -p "\$SITEDIR"

if [ $(versionCompare ${CARLA_VER} 9.9) -eq 1 ]
then
	echo "/opt/carla-simulator/PythonAPI/carla/dist/carla-${CARLA_VERSION}-py3.7-linux-x86_64.egg\n/opt/carla-simulator/PythonAPI/carla/" > "\$SITEDIR/carla.pth"
else
	echo "/opt/carla-simulator/PythonAPI/carla/dist/carla-${CARLA_VERSION}-py3.5-linux-x86_64.egg\n/opt/carla-simulator/PythonAPI/carla/" > "\$SITEDIR/carla.pth"
fi


SITEDIR=\$(python2 -c 'import site; site._script()' --user-site)
mkdir -p "\$SITEDIR"
echo "/opt/carla-simulator/PythonAPI/carla/dist/carla-${CARLA_VERSION}-py2.7-linux-x86_64.egg\n/opt/carla-simulator/PythonAPI/carla/" > "\$SITEDIR/carla.pth"

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

rm prerm

#Removing Carla library from site-packages
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

rm copyright
cp ../LICENSE ./copyright

#Updating debian/Changelog
awk '{sub(/UNRELEASED/,"stable")}1' changelog > tmp && mv tmp changelog
awk '{sub(/unstable/,"stable")}1' changelog > tmp && mv tmp changelog
cd ..

dpkg-buildpackage -uc -us -b #building debian package

#install debian package using "sudo dpkg -i ../carla-simulator_<version>_amd64.deb"

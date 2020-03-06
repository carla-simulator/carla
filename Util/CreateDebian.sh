#!/bin/sh

#This script builds debian package for CARLA
#Tested with Ubuntu 14.04, 16.04, 18.04 and 19.10

sudo apt-get install build-essential dh-make

#Adding maintainer name
DEBFULLNAME=Carla\ Simulator\ Team
export DEBFULLNAME

#replace CARLA_VERSION with your required carla-<version>
CARLA_VERSION=0.9.8
CARLA_DIR=carla-${CARLA_VERSION}

#replace the url with your carla release url
CARLA_RELEASE_REPO=http://carla-assets-internal.s3.amazonaws.com/Releases/Linux/CARLA_${CARLA_VERSION}.tar.gz

#replace the url with your required additional maps url
#if you do not have additional map then comment line 21, 22, 40, 41
ADDITIONALMAPS=http://carla-assets-internal.s3.amazonaws.com/Releases/Linux/AdditionalMaps_${CARLA_VERSION}.tar.gz
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

chmod +x "/opt/carla/CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping"
"/opt/carla/CarlaUE4/Binaries/Linux/CarlaUE4-Linux-Shipping" CarlaUE4 $@
EOF


rm Makefile

#Making debian package to install Carla in /opt folder
cat >> Makefile <<EOF
binary:
	# we are not going to build anything

install:
	mkdir -p \$(DESTDIR)/opt/carla/bin
	cp  CarlaUE4.sh \$(DESTDIR)/opt/carla/bin
	cp ImportAssets.sh \$(DESTDIR)/opt/carla
	cp -r CarlaUE4 \$(DESTDIR)/opt/carla
	cp -r Engine \$(DESTDIR)/opt/carla
	cp -r Import \$(DESTDIR)/opt/carla
	cp -r PythonAPI \$(DESTDIR)/opt/carla
EOF

dh_make -e carla.simulator@gmail.com --indep --createorig -y  #to create necessary file structure for debian packaging

cd debian/

#removing unnecessary files
rm ./*.ex
rm ./*.EX

rm control

#Adding package dependencies(Package will install them itself) and description
cat >> control <<EOF
Source: carla
Section: simulator
Priority: optional
Maintainer: Carla Simulator Team <carla.simulator@gmail.com>
Build-Depends: debhelper (>= 9)
Standards-Version: ${CARLA_VERSION}
Homepage: http://carla.org/

Package: carla
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
echo "/opt/carla/PythonAPI/carla/dist/${CARLA_DIR}-py3.5-linux-x86_64.egg\n/opt/carla/PythonAPI/carla/" > "\$SITEDIR/carla.pth"

SITEDIR=\$(python2 -c 'import site; site._script()' --user-site)
mkdir -p "\$SITEDIR"
echo "/opt/carla/PythonAPI/carla/dist/${CARLA_DIR}-py2.7-linux-x86_64.egg\n/opt/carla/PythonAPI/carla/" > "\$SITEDIR/carla.pth"

chmod +x /opt/carla/bin/CarlaUE4.sh

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

#install debian package using "sudo dpkg -i ../carla_<version>_amd64.deb"

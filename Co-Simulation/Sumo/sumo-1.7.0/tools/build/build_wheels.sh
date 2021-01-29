#!/bin/bash
yum-config-manager --add-repo=https://download.opensuse.org/repositories/home:/behrisch/CentOS_7/
yum install -y --nogpgcheck cmake3 ccache xerces-c-devel proj-devel fox16-devel bzip2-devel gl2ps-devel swig3
/opt/python/cp38-cp38/bin/pip install scikit-build

mkdir -p $HOME/.ccache
echo "hash_dir = false" >> $HOME/.ccache/ccache.conf
echo "base_dir = /github/workspace/_skbuild/linux-x86_64-3.8" >> $HOME/.ccache/ccache.conf
/opt/python/cp38-cp38/bin/python tools/build/setup-sumo.py --cmake-executable cmake3 -j 8 bdist_wheel
mv dist/sumo-* `echo dist/sumo-* | sed 's/cp38-cp38/py2.py3-none/'`
auditwheel repair dist/sumo*.whl
for py in /opt/python/*; do
    rm dist/*.whl
    $py/bin/pip install scikit-build
    pminor=`echo $py | sed 's,/opt/python/cp3,,;s/-.*//'`
    echo "base_dir = /github/workspace/_skbuild/linux-x86_64-3.${pminor}" >> $HOME/.ccache/ccache.conf
    $py/bin/python tools/build/setup-sumo.py --cmake-executable cmake3 -j 8 bdist_wheel
    $py/bin/python tools/build/setup-libsumo.py bdist_wheel
    auditwheel repair dist/libsumo*.whl
done

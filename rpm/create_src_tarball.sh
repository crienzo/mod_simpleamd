#!/bin/sh

MOD_SIMPLEAMD_DIR=`pwd`

if [ -z "$RPMBUILD_DIR" ]; then
  RPMBUILD_DIR=$HOME/rpmbuild
fi

mkdir -p $RPMBUILD_DIR
pushd $RPMBUILD_DIR
(mkdir -p SOURCES BUILD BUILDROOT RPMS SRPMS SPECS)
popd

tar --exclude='.git' -czf $RPMBUILD_DIR/SOURCES/mod_simpleamd.tar.gz -C $MOD_SIMPLEAMD_DIR .
cp rpm/mod_simpleamd.spec $RPMBUILD_DIR/SPECS


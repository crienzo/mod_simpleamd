#!/bin/bash

if [ -z "$RPMBUILD_DIR" ]; then
  RPMBUILD_DIR=$HOME/rpmbuild
fi

rpmbuild --define "_topdir $RPMBUILD_DIR" \
	-ba $RPMBUILD_DIR/SPECS/mod_simpleamd.spec


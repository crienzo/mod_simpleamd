mod_simpleamd: FreeSWITCH answering machine detection module using a simple energy-based VAD

To build and install from source:
```
sudo yum install freeswitch-devel
sudo yum install simpleamd-devel
./bootstrap.sh
PKG_CONFIG_PATH=/usr/share/freeswitch/pkgconfig bash -c './configure'
make
sudo make install
```

To create RPM package
```
rpm/create_src_tarball.sh
rpm/build_rpm.sh
```

RPM package will be in ~rpmbuild/RPMS/...


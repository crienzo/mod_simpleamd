mod_simpleamd: FreeSWITCH answering machine detection module using a simple energy-based VAD

To build and install from source:
sudo yum install freeswitch-devel freeswitch
sudo yum install simpleamd-devel simpleamd
./bootstrap.sh
PKG_CONFIG_PATH=/usr/share/freeswitch/pkgconfig bash -c './configure'
make
make install


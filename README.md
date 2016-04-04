janus-gateway-mplugin
==========================
[janus-gateway](https://github.com/meetecho/janus-gateway) custom plugin.


Overview
--------
This plugin is based on native `janus` audiobridge plugin. It does the conference and record the audio from room, but each participant is recorded in a different stream. The audio file is genereated using ffmpeg-lib/libav and the output file is a mkv with multiple streams.

Building
--------
If you got janus-gateway-mplugin from the git repository, you will first need to run the included `autogen.sh` script to generate the `configure` script.  The prefix is used to locate janus development files.

```
./autogen.sh
./configure  --prefix=/opt/janus
make
make install
```

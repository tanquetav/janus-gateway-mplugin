ACLOCAL_AMFLAGS = -I m4
JANUS_PATH=$(exec_prefix)
CFLAGS = $(shell pkg-config --cflags glib-2.0) -I$(JANUS_PATH)/include/janus $(shell pkg-config --cflags libavformat libavutil libswscale) 
CXXFLAGS = $(CFLAGS)
lib_LTLIBRARIES = libjanus_audiobridge2.la
libjanus_audiobridge2_la_SOURCES = libjanus_audiobridge2.c recorder.c
libjanus_audiobridge2_la_LDFLAGS = -version-info 0:0:0 $(shell pkg-config --libs glib-2.0) -L$(JANUS_PATH)/lib 

LIBS = $(shell pkg-config --libs glib-2.0) $(shell pkg-config --libs libavformat libavutil libswscale) 

confdir = $(exec_prefix)/etc/janus
libdir = $(exec_prefix)/lib/janus/plugins
conf_DATA = conf/janus.plugin.audiobridge2.cfg.sample

##
# Configuration
##

config:
	$(MKDIR_P) $(DESTDIR)$(confdir)
	$(foreach config,$(conf_DATA),cp "$(CURDIR)/$(config)" "$(DESTDIR)$(confdir)/$(notdir $(basename $(config) .sample))";)

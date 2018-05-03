include /data/workspace/dvda-author/mk/FLAC.global.mk

FLAC_MAKESPEC=auto
FLAC_CONFIGSPEC=lib
FLAC_DEPENDENCY=/data/workspace/dvda-author/depconf/libogg.depconf 
FLAC_TARGETLIB=libFLAC.a

/data/workspace/dvda-author/depconf/FLAC.depconf: $(FLAC_DEPENDENCY)
	$(call depconf,FLAC,"","",CFLAGS=)

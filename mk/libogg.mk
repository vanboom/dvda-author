include /data/workspace/dvda-author/mk/libogg.global.mk

libogg_MAKESPEC=auto
libogg_CONFIGSPEC=lib
libogg_TARGETLIB=libogg.a

/data/workspace/dvda-author/depconf/libogg.depconf: $(libogg_DEPENDENCY)
	$(call depconf,libogg)


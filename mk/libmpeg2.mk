include /home/fab/Dev/dvda-author/mk/libmpeg2.global.mk

libmpeg2_MAKESPEC=auto
libmpeg2_CONFIGSPEC=exe
libmpeg2_TESTBINARY=mpeg2dec$(EXEEXT)

/home/fab/Dev/dvda-author/depconf/libmpeg2.depconf: $(libmpeg2_DEPENDENCY)
	$(call depconf,libmpeg2)




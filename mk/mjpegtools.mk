include /data/workspace/dvda-author/mk/mjpegtools.global.mk

mjpegtools_MAKESPEC=auto
mjpegtools_CONFIGSPEC=exe
mjpegtools_TESTBINARY=mplex

/data/workspace/dvda-author/depconf/mjpegtools.depconf: $(mjpegtools_DEPENDENCY)
	$(call depconf,mjpegtools)  
	 

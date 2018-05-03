include /data/workspace/dvda-author/mk/ImageMagick.global.mk

ImageMagick_MAKESPEC=auto
ImageMagick_CONFIGSPEC=exe
ImageMagick_TESTBINARY=convert

/data/workspace/dvda-author/depconf/ImageMagick.depconf: $(ImageMagick_DEPENDENCY)
	$(call depconf,ImageMagick)

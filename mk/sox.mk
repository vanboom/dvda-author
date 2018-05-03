include /data/workspace/dvda-author/mk/sox.global.mk

sox_MAKESPEC=manual
ifeq "yes" "yes"
ifeq "yes" "yes"
ifeq "yes" "yes"

/data/workspace/dvda-author/depconf/sox.depconf: $(sox_DEPENDENCY)
	@if test  "$(MAYBE_sox)" != ""; then
		if test -d "/data/workspace/dvda-author/$(MAYBE_sox)" ; then
			cp "/data/workspace/dvda-author/sox-libs" "/data/workspace/dvda-author/$(MAYBE_sox)/sox-libs.in"
			else
			echo "/data/workspace/dvda-author/$(MAYBE_sox) is not a directory"
			exit -1
		fi
	fi
	$(call configure_lib_package,sox,/usr/bin/autoreconf,-if,CFLAGS=)
else
/data/workspace/dvda-author/depconf/sox.depconf: 
	echo "Please install libtoolize"
	exit -1

endif
else
/data/workspace/dvda-author/depconf/sox.depconf:
	echo "Please install automake"
	exit -1

endif
else
/data/workspace/dvda-author/depconf/sox.depconf:
	echo "Please install autoconf"
	exit -1
endif

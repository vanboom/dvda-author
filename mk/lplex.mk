include /data/workspace/dvda-author/mk/lplex.global.mk

lplex_MAKESPEC=auto
lplex_CONFIGSPEC=exe
lplex_DEPENDENCY += /data/workspace/dvda-author/depconf/FLAC.depconf Makefile
lplex_TESTBINARY=lplex

ifeq "yes" "yes"
ifeq "yes" "yes"
ifeq "yes" "yes"

/data/workspace/dvda-author/depconf/lplex.depconf: $(lplex_DEPENDENCY)
	 $(if $(MAYBE_lplex),
	  if test -d /data/workspace/dvda-author/$(MAYBE_lplex) ; then
	   cd /data/workspace/dvda-author/$(MAYBE_lplex) && $(SHELL) -c "autoreconf -if -Im4 -Iredist" 
	    if test "$$?" = "0"; then 
	    cd /data/workspace/dvda-author
	   else 
	    echo "autoreconf failed for lplex"
	    exit -1
	   fi
	   mkdir -p /data/workspace/dvda-author/$(MAYBE_lplex) && cd /data/workspace/dvda-author/$(MAYBE_lplex)
	   $(SHELL) /data/workspace/dvda-author/$(MAYBE_lplex)/configure  $(CONFIGURE_lplex_FLAGS) && $(MAKE) $(PARALLEL) && $(MAKE) install
	   if test "$$?" = "0"; then touch /data/workspace/dvda-author/depconf/lplex.depconf; fi
	   cd /data/workspace/dvda-author
	  fi)
	$(call index,lplex,$(EXEEXT),binary)
else
/data/workspace/dvda-author/depconf/lplex.depconf: 
	echo "Please install libtoolize"
	exit -1


endif
else
/data/workspace/dvda-author/depconf/lplex.depconf: 
	echo "Please install automake"
	exit -1

endif
else

/data/workspace/dvda-author/depconf/lplex.depconf: 
	echo "Please install autoconf"
	exit -1
endif

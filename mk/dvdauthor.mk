include /data/workspace/dvda-author/mk/dvdauthor.global.mk

dvdauthor_MAKESPEC=manual
dvdauthor_TESTBINARY=dvdauthor

ifeq "yes" "yes"
ifeq "yes" "yes"
ifeq "yes" "yes"

/data/workspace/dvda-author/depconf/dvdauthor.depconf: Makefile
	@mkdir -p /data/workspace/dvda-author/$(MAYBE_dvdauthor)/srcm4
	 cp -f /data/workspace/dvda-author/m4.extra.dvdauthor/*  /data/workspace/dvda-author/$(MAYBE_dvdauthor)/srcm4
	$(if $(MAYBE_dvdauthor),
	  if test -d /data/workspace/dvda-author/$(MAYBE_dvdauthor) ; then
	   cd /data/workspace/dvda-author/$(MAYBE_dvdauthor) && $(SHELL) -c "aclocal -Isrcm4 && autoheader && automake -acf && autoconf -Isrcm4" 
	    if test "$$?" = "0"; then 
	    cd /data/workspace/dvda-author
	   else 
	    echo "autoreconf failed for dvdauthor"
	    exit -1
	   fi
	   mkdir -p /data/workspace/dvda-author/$(MAYBE_dvdauthor) && cd /data/workspace/dvda-author/$(MAYBE_dvdauthor)
	   $(SHELL) /data/workspace/dvda-author/$(MAYBE_dvdauthor)/configure  $(CONFIGURE_dvdauthor_FLAGS) && $(MAKE) $(PARALLEL) && $(MAKE) install
	   if test "$$?" = "0"; then touch /data/workspace/dvda-author/depconf/dvdauthor.depconf; fi
	   cd /data/workspace/dvda-author
	  fi)
	$(call index,dvdauthor,$(EXEEXT),binary)
else
/data/workspace/dvda-author/depconf/dvdauthor.depconf: 
	echo "Please install libtoolize"
	exit -1


endif
else
/data/workspace/dvda-author/depconf/dvdauthor.depconf: 
	echo "Please install automake"
	exit -1

endif
else

/data/workspace/dvda-author/depconf/dvdauthor.depconf: 
	echo "Please install autoconf"
	exit -1
endif

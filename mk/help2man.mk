include /data/workspace/dvda-author/mk/help2man.global.mk

help2man_MAKESPEC=auto
help2man_CONFIGSPEC=exe
help2man_TESTBINARY=help2man

/data/workspace/dvda-author/depconf/help2man.depconf: 
	$(call depconf,help2man)

OPTS =  -DCLI_CSCACHE -DSRV_CSCACHE -DCHECK_NEXTDCW -DHTTP_SRV -DMGCAMD_SRV -DCCCAM_SRV -DCCCAM_CLI -DRADEGAST_CLI -DSID_FILTER -DNEWCACHE -DDONT_CHECK_PEER_ADDR

ifeq ($(target),x32)
  CC        = gcc
  OUTPUT	= x32/
  CFLAGS	= -s -ggdb3 -m32 -O3 -I. $(OPTS)
  LFLAGS	= $(CFLAGS)
else
ifeq ($(target),x64)
  CC        = gcc
  OUTPUT	= x64/
  CFLAGS	= -s -ggdb3 -m64 -O3 -I. $(OPTS)
  LFLAGS	= $(CFLAGS)
else
ifeq ($(target),ppc)
  CC        = powerpc-tuxbox-linux-gnu-gcc
  OUTPUT	= ppc/
  CFLAGS	= -s -ggdb3 -O2 $(OPTS) -DINOTIFY
  LFLAGS	= $(CFLAGS)
else
ifeq ($(target),mips)
  CC        = mips-openwrt-linux-gcc
  OUTPUT	= mips/
  CFLAGS	= -s -ggdb3 -O2 $(OPTS)
  LFLAGS	= $(CFLAGS)
else
ifeq ($(target),openwrt)
  CC        = /root/Desktop/OpenWrt-SDK-ixp4xx-2.6-for-Linux-i686/staging_dir_armeb/bin/armeb-linux-gcc
  OUTPUT	= openwrt/
  CFLAGS	= -s -ggdb3 -O2 $(OPTS)
  LFLAGS	= $(CFLAGS)
else
ifeq ($(target),sh4)
  CC        = /opt/STM/STLinux-2.4/devkit/sh4/bin/sh4-linux-gcc
  OUTPUT	= sh4/
  CFLAGS	= -s -ggdb3 -O2 $(OPTS) -DST_7201  -DST_OSLINUX  -DARCHITECTURE_ST40
  LFLAGS	= $(CFLAGS)
else
ifeq ($(target),win32)
  CC        = i586-mingw32msvc-gcc
  OUTPUT	= win32/
  CFLAGS	= -s -ggdb3 -O2 -DWIN32 $(OPTS)
  LFLAGS	= $(CFLAGS)
else
ifeq ($(target),64)
  CC        = gcc
  OUTPUT	= x/
  CFLAGS	= -s -ggdb3 -m64 -O3 -I. $(OPTS) -D_GNU_SOURCE -DSIG_HANDLER -DDEBUG_NETWORK ## -D_FORTIFY_SOURCE=0 
  LFLAGS	= $(CFLAGS)
else
  CC        = gcc
  OUTPUT	= x/
  CFLAGS	= -s -ggdb3 -O3 -Wall -I. $(OPTS) -D_GNU_SOURCE -DSIG_HANDLER -DDEBUG_NETWORK ## -D_FORTIFY_SOURCE=0 
  LFLAGS	= $(CFLAGS)
endif
endif
endif
endif
endif
endif
endif
endif

ifndef name
	AOUT	= $(OUTPUT)multics
else
	AOUT	= $(name)
endif


OBJECTS = $(OUTPUT)sha1.o $(OUTPUT)des.o $(OUTPUT)md5.o $(OUTPUT)convert.o $(OUTPUT)tools.o $(OUTPUT)threads.o $(OUTPUT)debug.o \
	$(OUTPUT)sockets.o $(OUTPUT)msg-newcamd.o $(OUTPUT)msg-cccam.o $(OUTPUT)msg-radegast.o $(OUTPUT)parser.o $(OUTPUT)config.o \
	$(OUTPUT)ecmdata.o $(OUTPUT)httpserver.o $(OUTPUT)main.o

link: $(OBJECTS)
	$(CC) $(LFLAGS) -o $(AOUT) $(OBJECTS) -lpthread

%.o: ../%.c Makefile common.h httpserver.h config.h
	$(CC) -c $(CFLAGS) $< -o $@

$(OUTPUT)main.o: main.c Makefile common.h httpserver.h config.h clustredcache.c dcwfilter.c pipe.c \
	th-ecm.c th-cfg.c th-dns.c th-date.c th-srv.c \
	srv-cccam.c srv-newcamd.c srv-newcamd.c srv-radegast.c \
	cli-common.c cli-cccam.c cli-newcamd.c cli-radegast.c

encrypt/encrypt:
	cd encrypt && $(MAKE) encrypt

clean:
	-rm $(OUTPUT)*

cleanall:
	$(MAKE) clean
	$(MAKE) target=x64 clean
	$(MAKE) target=x32 clean
	$(MAKE) target=ppc clean
	$(MAKE) target=mips clean
	$(MAKE) target=sh4 clean
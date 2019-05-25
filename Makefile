
#
#	Makefile: gps logger makefile file and release utilities.
#	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
#	refer to the associated LICENSE file for licensing terms/conditions.
#

################################################################################
# release building: make sure that the emulator is built on each target
# machine using the 'relprep' target, then scp release/emulator* over to
# the local machine's release directory, and do a make release: this puts
# all the files (including the various emulator* files) together into a
# single zip file.
################################################################################

# profiling:
# CC=gcc
# COPTS=-O3 -pg -pipe -Wall -Wextra -Werror #-Wconversion
# CDEFS=-DEMULATE -DHARDWARE_24# -DDEBUG

# building:
CC = gcc43
COPTS = -O3 -pipe \
		-Wall -Wextra \
		-Wformat-y2k -Wno-format-extra-args -Wno-format-zero-length -Wformat-nonliteral -Wformat-security -Wformat=2 \
		-Wuninitialized -Winit-self -Wswitch-default -Wswitch-enum -Wunused -Wstrict-aliasing=2 \
		-Wdeclaration-after-statement -Wshadow -Wpointer-arith -Wbad-function-cast \
		-Wcast-qual -Wcast-align -Wwrite-strings -Waggregate-return -Wstrict-prototypes -Wold-style-definition \
		-Wmissing-prototypes -Wmissing-declarations -Wmissing-noreturn -Wmissing-format-attribute -Wredundant-decls \
		-Wlong-long -Wdisabled-optimization -fgcse-sm -fgcse-las -fgcse-after-reload -fweb \
		#-Werror -Wconversion
COTHR=-funsafe-loop-optimizations -Wunsafe-loop-optimizations -Wvla -Wlogical-op -Wstrict-overflow -Wstrict-overflow=5 \
		-ftree-loop-linear -ftree-loop-ivcanon -fivopts -ftree-vectorize -fvariable-expansion-in-unroller
CDEFS=-DEMULATE -DHARDWARE_24 -DINLINE

# debugging:
COPTS += -g
CDEFS += -DDEBUG

# profiling:
# CC = gcc
# COPTS += -pg
# COTHR =

CFLAGS=$(COPTS) $(COTHR) $(CDEFS)
LFLAGS=$(COPTS) -static

SPLINT=splint
SFLAGS_COMMON=-DSPLINT -booltype boolean +unixlib
SFLAGS_CHECKS=$(SFLAGS_COMMON) \
			+checks -mustmod -globuse +charindex -mustdefine
SFLAGS_STRICT=$(SFLAGS_COMMON) \
			+strict -mustmod -globuse +charindex -mustdefine
SFLAGS=$(SFLAGS_CHECKS)

################################################################################

VERSION=$(shell sed -n 's/^.*VERSION[^"]*\"\([^"]*\)\"/\1/p' < common.h)
SYSTEM=$(shell uname -srm | sed 's/ /_/g' | tr A-Z a-z)
CDEFS+=-DSYSTEM=\"$(SYSTEM)\"

ifneq (,$(findstring linux,$(SYSTEM)))
SFLAGS+=-Dlinux
endif

################################################################################

TARGET_EMU=gps_logger
TARGET_LZD=gps_lzdecode
TARGET_LZE=gps_lzencode
TARGET_CVT=gps_convert
TARGET_DOC=gps_logger.pdf

IMAGE=sdfiles/fat.img

RELEASE_EMU=release/emulator-$(SYSTEM)

################################################################################

RELEASE_NAM=gps_logger_mg-$(VERSION)

RELEASE_SRC=\
	"ARM Flash Debug - V1.0/gps_logger.hex",firmware-sparkfun_gpslog10-debug.hex:0644: \
	"ARM Flash Release - V1.0/gps_logger.hex",firmware-sparkfun_gpslog10-release.hex:0644: \
	"ARM Flash Debug - V2.4/gps_logger.hex",firmware-sparkfun_gpslog24-debug.hex:0644: \
	"ARM Flash Release - V2.4/gps_logger.hex",firmware-sparkfun_gpslog24-release.hex:0644: \
	"main_lzdecode.c",utility-lzari-unpack.c:0644:k \
	"scripts/unpack.pl",utility-csvbin-unpack.pl:0755:k \
	"release/examples.zip",examples.zip:0644: \
	"gps_logger.pdf",userguide.pdf:0644: \
	"support/release/README",README:0644:k \
	"support/release/LICENSE",LICENSE:0644:k \
	"CHANGES",CHANGES:0644: \

################################################################################

OBJS_APPLICATION=\
	main.o \
	main_config.o \
	main_command.o \
	main_debug.o \
	mod_gps.o \
	mod_gpsformat.o \
	mod_gpsoutput.o \
	mod_gpsinput.o \
	mod_gpsprocess.o \
	mod_gpscapture.o \
	mod_xferymodem.o \
	mod_packlzari.o \
	mod_command.o \
	mod_config.o \
	mod_filecache.o \
	mod_filefatfs.o \
	mod_filespisd.o \
	mod_util.o \

INLI_APPLICATION=\
	mod_util.i \
	mod_lpc21XXhal.i

OBJS_HARDWARE_EMU=\
	mod_lpc21XXemu.o

OBJS_HARDWARE_HAL=\
	mod_lpc21XXhal.o

OBJS_EMU=$(OBJS_APPLICATION) $(OBJS_HARDWARE_EMU)
SRCS_EMU=$(OBJS_EMU:.o=.c)
INLI_EMU=$(INLI_APPLICATION)
LIBS_EMU=-lpthread

OBJS_HAL=$(OBJS_APPLICATION) $(OBJS_HARDWARE_HAL)
SRCS_HAL=$(OBJS_HAL:.o=.c)

OBJS_LZD=main_lzdecode.o
SRCS_LZD=$(OBJS_LZD:.o=.c)
LIBS_LZD=

OBJS_LZE=main_lzencode.o
SRCS_LZE=$(OBJS_LZE:.o=.c)
LIBS_LZE=

################################################################################

all: .depend $(TARGET_EMU) $(TARGET_LZD) $(TARGET_LZE)

$(TARGET_EMU): $(OBJS_EMU)
	$(CC) $(LFLAGS) -Wl,-Map,gps_logger.map -o $@ $(OBJS_EMU) $(LIBS_EMU)

$(TARGET_LZD): $(OBJS_LZD)
	$(CC) $(LFLAGS) -o $@ $(OBJS_LZD) $(LIBS_LZD)

$(TARGET_LZE): $(OBJS_LZE)
	$(CC) $(LFLAGS) -o $@ $(OBJS_LZE) $(LIBS_LZE)

main_lzdecode.c: mod_packlzari.c
	scripts/preproc.pl -DPACKLZARI_STANDALONE -DPACKLZARI_DECODE -UPACKLZARI_ENCODE \
			-UPACKLZARI_N -UPACKLZARI_F -UPACKLZARI_T \
			< mod_packlzari.c | unexpand -a -t 4 > main_lzdecode.c

main_lzencode.c: mod_packlzari.c
	scripts/preproc.pl -DPACKLZARI_STANDALONE -DPACKLZARI_ENCODE -UPACKLZARI_DECODE \
			< mod_packlzari.c | unexpand -a -t 4 > main_lzencode.c

.depend: $(INLI_EMU)
	-rm -f .depend
	for o in $(OBJS_EMU:.o=.c); do \
		$(CC) -M $(CFLAGS) $$o; \
	done > .depend

-include .depend

################################################################################

mod_util.h: mod_util.i
mod_util.i: mod_util.c
	scripts/preproc.pl -DINLINE -IINLINE -c < mod_util.c > mod_util.i

mod_lpc21XXhal.h: mod_lpc21XXhal.i
mod_lpc21XXhal.i: mod_lpc21XXhal.c
	scripts/preproc.pl -DINLINE -IINLINE -c < mod_lpc21XXhal.c > mod_lpc21XXhal.i

################################################################################

gps_logger.pdf: gps_logger.doc
	@echo fatal: you need to manually convert \"gps_logger.doc\" to \"gps_logger.pdf\"
	@false

################################################################################

.PHONY: splint_emulator \
		splint_firmware_debug_10 splint_firmware_release_10 \
		splint_firmware_debug_24 splint_firmware_release_24 \
		splint_lzdecode splint_lzencode splint

splint_emulator: $(SRCS_EMU)
	$(SPLINT) $(CDEFS) -I. $(SFLAGS) \
		$(SRCS_EMU)

splint_firmware_debug_10: $(SRCS_HAL)
	$(SPLINT) -DDEBUG -DHARDWARE_10 -DINLINE -I. -Isupport/include $(SFLAGS) \
		$(SRCS_HAL)

splint_firmware_release_10: $(SRCS_HAL)
	$(SPLINT) -DRELEASE -DHARDWARE_10 -DINLINE -I. -Isupport/include $(SFLAGS) \
		$(SRCS_HAL)

splint_firmware_debug_24: $(SRCS_HAL)
	$(SPLINT) -DDEBUG -DHARDWARE_24 -DINLINE -I. -Isupport/include $(SFLAGS) \
		$(SRCS_HAL)

splint_firmware_release_24: $(SRCS_HAL)
	$(SPLINT) -DRELEASE -DHARDWARE_24 -DINLINE -I. -Isupport/include $(SFLAGS) \
		$(SRCS_HAL)

splint_lzdecode: $(SRCS_LZD)
	$(SPLINT) -DPACKLZARI_DECODE -DPACKLZARI_STANDALONE $(SFLAGS) \
		$(SRCS_LZD)

splint_lzencode: $(SRCS_LZE)
	$(SPLINT) -DPACKLZARI_ENCODE -DPACKLZARI_STANDALONE $(SFLAGS) \
		$(SRCS_LZE)

splint: splint_emulator \
		splint_firmware_release_10 splint_firmware_debug_10 \
		splint_firmware_release_24 splint_firmware_debug_24 \
		splint_lzdecode splint_lzencode

################################################################################

.PHONY: test

test: $(TARGET_EMU) $(IMAGE)
	./$(TARGET_EMU) -i $(IMAGE) -- --cfg_debug=diag
	./$(TARGET_EMU) -i $(IMAGE) -d . -- --cfg_debug=diag

################################################################################

.PHONY: release relprep changes

changes:
	svn up && svn log | scripts/changes.pl > CHANGES

relprep: $(RELEASE_EMU)

$(RELEASE_EMU): $(TARGET_EMU)
	mkdir -p release
	cp -f $(TARGET_EMU) $(RELEASE_EMU)

relexamples: $(TARGET_EMU) $(TARGET_LZD)
	mkdir -p release/examples
	rm -rf release/examples.zip
	(cd release/examples; ../../scripts/genexamples.sh ../../samples/raw/GPS00001.TXT gps00000)
	(cd release/examples; ../../scripts/genexamples.sh ../../samples/raw/GPS00002.TXT gps00001)
	(cd release/examples; zip -9 -q -r ../examples.zip *)
	rm -rf release/examples

release: relexamples relprep $(TARGET_EMU) $(TARGET_LZD) gps_logger.pdf changes
	rm -rf release/$(RELEASE_NAM) release/$(RELEASE_NAM).zip
	mkdir -p release/$(RELEASE_NAM)
	for i in release/emulator-*; do \
		install -m 0755 -o public -g public "$$i" "release/$(RELEASE_NAM)/"; \
	done
	for i in $(RELEASE_SRC); do \
		s=`echo $$i|sed 's/,.*//g'`;\
		d=`echo $$i|sed 's/^.*,\([^\:]*\).*/\1/g'`;\
		p=`echo $$i|sed 's/^.*:\([^\:]*\):.*/\1/g'`;\
		f=`echo $$i|sed 's/^.*://g'`;\
		if [ "$$f" = "k" ]; then \
			scripts/kwsubsts.pl common.h < "$$s" > .buildtmp && \
				install -m $$p -o public -g public .buildtmp "release/$(RELEASE_NAM)/$$d" && \
				rm -f .buildtmp ; \
		else \
			install -m $$p -o public -g public "$$s" "release/$(RELEASE_NAM)/$$d"; \
		fi; \
	done
	(cd release; zip -9 -q -r $(RELEASE_NAM).zip $(RELEASE_NAM)/)
	cp -f $(TARGET_DOC) release/$(RELEASE_NAM).pdf
	cp -f support/release/RELNOTE release/$(RELEASE_NAM).RELNOTE
	cp -f CHANGES release/$(RELEASE_NAM).CHANGES
	ls -l release/$(RELEASE_NAM).*

################################################################################

.PHONY: clean distclean

clean:
	rm -f .depend .buildtmp \
		$(OBJS_EMU) $(OBJS_LZD) $(OBJS_LZE) $(TARGET_EMU) $(TARGET_LZD) $(TARGET_LZE) $(SRCS_LZD) $(SRCS_LZE) \
		*.i *.o *.core *.map *.TMP *.gmon

distclean: clean
	rm -f $(IMAGE)
	rm -f GPS*.TXT
	rm -rf ARM\ Flash*
	rm -rf release

################################################################################

.PHONY: fat_create fat_mount fat_umount

$(IMAGE): fat_create

fat_create_8m:
	bzcat sdfiles/fat_8m.img.bz2 > $(IMAGE)

fat_create_128m:
	bzcat sdfiles/fat_128m.img.bz2 > $(IMAGE)

fat_create_256m:
	bzcat sdfiles/fat_256m.img.bz2 > $(IMAGE)

fat_create: fat_create_8m

fat_mount:
	-mdconfig -a -t vnode -f $(IMAGE) -u 1
	-fsck_msdosfs /dev/md1s1
	-mount -t msdosfs /dev/md1s1 /mnt/fat

fat_umount:
	-umount /mnt/fat
	-mdconfig -d -u 1

################################################################################

.PHONY: tabfix

tabfix:
	@for i in *.h *.c Makefile; do \
		unexpand -a -t 4 < $$i > .tmp-tabfix && mv -f .tmp-tabfix $$i; \
	done

################################################################################


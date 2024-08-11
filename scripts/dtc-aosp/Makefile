# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
#
# This is based on the original scripts/dtc/Makefile, but has been revised to
# work with an insane submodule situation.

hostprogs-always-y := dtc-aosp fdtoverlay

dtc-aosp-objs	:= dtc.o flattree.o fstree.o data.o livetree.o treesource.o \
		   srcpos.o checks.o util.o
dtc-aosp-objs	+= dtc-lexer.lex.o dtc-parser.tab.o

# We're working with a submodule, so make these all relative to that.
dtc-aosp-objs	:= $(addprefix dtc/,$(dtc-aosp-objs))

HOSTCFLAGS_DTC := -I$(srctree)/$(src)/dtc -I$(srctree)/$(src)/dtc/libfdt
HOSTCFLAGS_DTC += -DNO_YAML

HOSTCFLAGS_FDT := -I$(srctree)/$(src)/dtc/libfdt

# The upstream project builds libfdt as a separate library.  We are choosing to
# instead directly link the libfdt object files into fdtoverlay.
libfdt-objs	:= fdt.o fdt_ro.o fdt_wip.o fdt_sw.o fdt_rw.o fdt_strerror.o fdt_empty_tree.o fdt_addresses.o fdt_overlay.o
libfdt		= $(addprefix dtc/libfdt/,$(libfdt-objs))
fdtoverlay-objs	:= $(libfdt) dtc/fdtoverlay.o dtc/util.o

HOSTCFLAGS_dtc/libfdt/fdt.o := $(HOSTCFLAGS_FDT)
HOSTCFLAGS_dtc/libfdt/fdt_ro.o := $(HOSTCFLAGS_FDT)
HOSTCFLAGS_dtc/libfdt/fdt_wip.o := $(HOSTCFLAGS_FDT)
HOSTCFLAGS_dtc/libfdt/fdt_sw.o := $(HOSTCFLAGS_FDT)
HOSTCFLAGS_dtc/libfdt/fdt_rw.o := $(HOSTCFLAGS_FDT)
HOSTCFLAGS_dtc/libfdt/fdt_strerror.o := $(HOSTCFLAGS_FDT)
HOSTCFLAGS_dtc/libfdt/fdt_empty_tree.o := $(HOSTCFLAGS_FDT)
HOSTCFLAGS_dtc/libfdt/fdt_addresses.o := $(HOSTCFLAGS_FDT)
HOSTCFLAGS_dtc/libfdt/fdt_overlay.o := $(HOSTCFLAGS_FDT)
HOSTCFLAGS_dtc/fdtoverlay.o := $(HOSTCFLAGS_FDT)
HOSTCFLAGS_dtc/util.o := $(HOSTCFLAGS_FDT)

HOSTCFLAGS_dtc/checks.o := $(HOSTCFLAGS_DTC)
HOSTCFLAGS_dtc/data.o := $(HOSTCFLAGS_DTC)
HOSTCFLAGS_dtc/dtc.o := $(HOSTCFLAGS_DTC)
HOSTCFLAGS_dtc/flattree.o := $(HOSTCFLAGS_DTC)
HOSTCFLAGS_dtc/fstree.o := $(HOSTCFLAGS_DTC)
HOSTCFLAGS_dtc/livetree.o := $(HOSTCFLAGS_DTC)
HOSTCFLAGS_dtc/srcpos.o := $(HOSTCFLAGS_DTC)
HOSTCFLAGS_dtc/treesource.o := $(HOSTCFLAGS_DTC)
HOSTCFLAGS_dtc/util.o := $(HOSTCFLAGS_DTC)

HOSTCFLAGS_dtc/dtc-lexer.lex.o := $(HOSTCFLAGS_DTC)
HOSTCFLAGS_dtc/dtc-parser.tab.o := $(HOSTCFLAGS_DTC)

# Dependencies on generated files need to be listed explicitly.
$(obj)/dtc/dtc-lexer.lex.o: $(obj)/dtc/dtc-parser.h

# Make the header output .h instead of .tab.h for bison, because aosp is broken.
%.h: %.y
	$(call if_changed,bison)

# Generated files need to be cleaned explicitly.
clean-files	:= dtc/dtc-lexer.lex.c dtc/dtc-parser.tab.c dtc/dtc-parser.h

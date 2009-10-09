# Makefile for the Open Source Camera Code Collection (osc-cc).
# Copyright (C) 2009 Supercomputing Systems AG
# 
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2.1 of the License, or (at your option)
# any later version.
# 
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License along
# with this library; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

# Disable make's built-in rules.
MAKE += -RL
# --no-print-directory
SHELL := $(shell which bash) -e -o pipefail

# Include the file generated by te configuration process.
-include .config
ifeq '$(filter .config, $(MAKEFILE_LIST))' ''
$(error Please configure the library using './configure' prior to compilation.)
endif

# .PHONY is broken on static pattern rules.
.PHONY: .FORCE

# Executable to create the static library
AR_host := ar -rcs
AR_target := bfin-uclinux-ar -rcs

# Modules to compile
#MODULES := Classification/Labeling
MODULES := $(shell find . -mindepth 2 -name Makefile -exec  dirname {} \;)

# Modes to compile this module in.
MODES := host target
MODES += $(addsuffix _dbg, $(MODES))

# Helper function to access stacked, eg. mode-dependent variables.
varnames = $(filter $(.VARIABLES), $(foreach i, $(shell seq 1 $(words $(subst _, , $(1)))), $(subst $() ,_,$(wordlist 1, $i, $(subst _, , $(1))))))
firstvar = $($(lastword $(call varnames, $(1))))
allvars = $(foreach i, $(call varnames, $(1)), $($i))

# Default target which builds all modules for the selected board configuration.
all: $(MODES) .FORCE;

reconfigure:
ifeq '$(CONFIG_PRIVATE_FRAMEWORK)' 'n'
	@ ! [ -e "oscar" ] || [ -h "oscar" ] && ln -sfn $(CONFIG_FRAMEWORK_PATH) oscar
endif
	@ ! [ -d "oscar" ] || $(MAKE) -C oscar config
	
# Targets to build in a specific build mode and create the library.
$(MODES): %: library/libosc-cc_%.a .FORCE;
library/libosc-cc_%.a: osc-cc_% $(addsuffix /%, $(MODULES))
	$(call firstvar, AR_$*) $@ $(addsuffix /build/*_$*.o, $(MODULES))

# Allow a module to be given on the command line to build that module.
$(MODULES): .FORCE
	$(MAKE) -C $@

# Call this as osc-cc_$(MODE) to only build the main osc-cc files.
osc-cc_%: .FORCE
	$(MAKE) -f Makefile_module $*

# Produce a target of the form "foo/%" for every directory foo that contains a Makefile
define subdir_target
$(1)/%: .FORCE
	$(MAKE) -C $(1) $$*
endef
SUBDIRS := $(sort $(patsubst %/, %, $(dir $(wildcard */Makefile))) $(MODULES))
$(foreach i, $(SUBDIRS), $(eval $(call subdir_target, $(i))))

# Do not try to rebuild any of the Makefiles.
$(sort $(MAKEFILE_LIST)):;

# Routing individual object file requests directly to the compile Makefile
%.o: .FORCE
	$(MAKE) -f Makefile_module $@

# Cleans the framework and all modules
clean: %: $(addsuffix /%, $(MODULES)) osc-cc_clean .FORCE
	rm -rf library/*.a

# Cleans everything not intended for source distribution
distclean: clean .FORCE
	rm -rf .config

# This captures all warnings generated while compiling the framework and sorts them by file.
warnings: .FORCE
	@ echo "Gathering all warnings, this may take a minute ..." >&2
	@ $(MAKE) clean &> /dev/null
	@ { $(MAKE) all; $(MAKE) doc; } 2>&1 > /dev/null | sed -rn 's,^([^:]+:[0-9]+): (warning|error): (.*)$$,\1 \3,pi' | sort | uniq | tee >(wc -l | sed -rn 's|[\t ]*||g; s|$$| warnings.|p')

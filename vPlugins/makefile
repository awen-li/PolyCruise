#############################################################################
# Author: Wen Li
# Date  : 1/31/2021
# Description: makefile for entry of module of project
##############################################################################

export CC = clang
export LD = clang

export CFLAGS  = -fPIC -g
export LDFLAGS = -fPIC -shared -lDynAnalyze

export INCLUDE = $(POLYC_PATH)/DynAnalyzer/include

SUBDIRS = debuf-overflow  decfi  dedivzero  deincmp  deinject  deint-overflow  deleak detypeerr

$(SUBDIRS):init
	for subdir in $(SUBDIRS);\
	do $(MAKE) -C $$subdir;\
	done

init:
	@if [ ! -d $(LIB_PATH) ]; then	mkdir $(LIB_PATH); fi
	@echo begin compile $(PWD)

.PHONY : clean
clean:
	for subdir in $(SUBDIRS);\
	do $(MAKE) -C $$subdir clean;\
	done
	@rm -rf lib

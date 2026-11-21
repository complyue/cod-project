repo := ../../..
OUT ?= $(repo)/build-cod-llvm
LLVM_CONFIG ?= $(TOOLCHAIN)/bin/llvm-config
library := libcod-llvm.a
sources := cod_llvm_commands.c cod_llvm_object.c

component_tools = $(abspath $(call find_tool,$(LLVM_CONFIG),LLVM_CONFIG))
component_flags = $(shell "$(tools)" --cflags)$(if $(filter 0,$(.SHELLSTATUS)),,$(error LLVM_CONFIG --cflags failed: $(tools)))

include ../archive.mk

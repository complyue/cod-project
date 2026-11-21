.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

TOOLCHAIN ?= $(repo)/build
CLANG ?= $(TOOLCHAIN)/bin/clang
LLVM_AR ?= $(TOOLCHAIN)/bin/llvm-ar
CFLAGS ?= -O2 -Wall -Wextra -Werror -pedantic-errors

quote = '$(subst ','"'"',$(1))'
find_tool = $(or $(shell command -v $(call quote,$(1))),$(error $(2) executable not found: $(1)))
clang := $(abspath $(call find_tool,$(CLANG),CLANG))
ar := $(abspath $(call find_tool,$(LLVM_AR),LLVM_AR))
tools := $(component_tools)
flags := $(component_flags)

ifeq ($(strip $(OUT)),)
$(error OUT must name a separate native component output directory)
endif
out := $(shell realpath -m -- $(call quote,$(OUT)))
toolchain_dirs := $(shell realpath -m -- $(repo)/build $(call quote,$(TOOLCHAIN)))
ifneq ($(filter $(toolchain_dirs) $(addsuffix /%,$(toolchain_dirs)),$(out)),)
$(error OUT must be outside the seed and LLVM build tree: $(out))
endif

archive := $(out)/$(library)
inputs := $(archive).inputs
objects := $(addprefix $(out)/,$(sources:.c=.o))
makefiles := $(MAKEFILE_LIST)

.PHONY: all FORCE
all: $(archive)

$(out):
	mkdir -p "$@"

$(inputs): FORCE | $(out)
	@printf '%s\n' $(call quote,$(clang)) $(call quote,$(ar)) \
		$(call quote,$(CPPFLAGS)) $(call quote,$(CFLAGS)) $(call quote,$(flags)) \
		$(call quote,$(tools)) $(call quote,$(sources)) > "$@.tmp"
	@if cmp -s "$@.tmp" "$@"; then rm "$@.tmp"; else mv "$@.tmp" "$@"; fi

$(objects): $(out)/%.o: %.c $(clang) $(tools) $(makefiles) $(inputs) | $(out)
	"$(clang)" $(CPPFLAGS) $(flags) -std=c11 $(CFLAGS) -MMD -MP -c "$<" -o "$@"

$(archive): $(objects) $(ar) $(makefiles) $(inputs)
	@set -eu; stage=$$(mktemp -d "$(out)/.archive.XXXXXX"); \
		trap 'rm -rf "$$stage"' EXIT; \
		"$(ar)" rcs "$$stage/$(library)" $(objects); \
		mv "$$stage/$(library)" "$@"

-include $(objects:.o=.d)

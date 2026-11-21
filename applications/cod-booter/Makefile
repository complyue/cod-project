.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

ifeq ($(filter grouped-target,$(.FEATURES)),)
$(error GNU Make 4.3 or newer is required)
endif

TOOLCHAIN ?= ../../build
COD_SEED ?= $(TOOLCHAIN)/bin/cod
LLVM_CONFIG ?= $(TOOLCHAIN)/bin/llvm-config
CLANG ?= $(TOOLCHAIN)/bin/clang
CLANGXX ?= $(TOOLCHAIN)/bin/clang++
OUT ?= ../../build-cod-booter

COD_SEED_FLAGS ?=
CFLAGS ?= -O1 -Wall -Wextra -Werror -pedantic-errors
CXXFLAGS ?= -O1
IRFLAGS ?= -O1

quote = '$(subst ','"'"',$(1))'
find_tool = $(or $(shell command -v $(call quote,$(1))),$(error $(2) executable not found: $(1)))

seed := $(abspath $(call find_tool,$(COD_SEED),COD_SEED))
llvm_config := $(abspath $(call find_tool,$(LLVM_CONFIG),LLVM_CONFIG))
clang := $(abspath $(call find_tool,$(CLANG),CLANG))
clangxx := $(abspath $(call find_tool,$(CLANGXX),CLANGXX))

ifeq ($(strip $(OUT)),)
$(error OUT must name a separate booter output directory)
endif
out := $(shell realpath -m -- $(call quote,$(OUT)))
toolchain_dirs := $(shell realpath -m -- ../../build $(call quote,$(TOOLCHAIN)))
ifneq ($(filter $(toolchain_dirs) $(addsuffix /%,$(toolchain_dirs)),$(out)),)
$(error OUT must be outside the seed and LLVM build tree: $(out))
endif

llvm_query = $(shell "$(llvm_config)" $(1))$(if $(filter 0,$(.SHELLSTATUS)),,$(error LLVM_CONFIG failed: $(llvm_config) $(1)))
llvm_components := core analysis native
llvm_cflags := $(call llvm_query,--cflags)
llvm_ldflags := $(call llvm_query,--link-shared --ldflags --libs $(llvm_components) --system-libs)
llvm_libfiles := $(call llvm_query,--link-shared --libfiles $(llvm_components))
llvm_libdir := $(call llvm_query,--libdir)

entry := src/CompilerCLI.cod
cod_sources := $(sort $(shell find src -type f -name '*.cod'))
native_sources := $(sort $(wildcard native/*.c))
generated_names := cod_providers.h cod_providers.c driver.ll runtime.cc
generated := $(addprefix $(out)/,$(generated_names))
c_objects := $(patsubst native/%.c,$(out)/native/%.o,$(native_sources)) $(out)/cod_providers.o
objects := $(c_objects) $(out)/driver.o $(out)/runtime.o

.PHONY: all FORCE
all: $(out)/cod-booter

$(out) $(out)/native:
	mkdir -p "$@"

# Preserve stamp mtimes when the effective inputs are unchanged.
$(out)/seed.inputs: FORCE | $(out)
	@printf '%s\n' $(call quote,$(seed)) $(call quote,$(COD_SEED_FLAGS)) \
		$(foreach source,$(cod_sources),$(call quote,$(source))) > "$@.tmp"
	@if cmp -s "$@.tmp" "$@"; then rm "$@.tmp"; else mv "$@.tmp" "$@"; fi

$(out)/native.inputs: FORCE | $(out)
	@printf '%s\n' $(call quote,$(clang)) $(call quote,$(clangxx)) $(call quote,$(llvm_config)) \
		$(call quote,$(CPPFLAGS)) $(call quote,$(CFLAGS)) $(call quote,$(CXXFLAGS)) $(call quote,$(IRFLAGS)) \
		$(call quote,$(LDFLAGS)) $(call quote,$(LDLIBS)) $(call quote,$(llvm_cflags)) \
		$(call quote,$(llvm_ldflags)) $(call quote,$(llvm_libfiles)) $(call quote,$(llvm_libdir)) \
		$(foreach source,$(native_sources),$(call quote,$(source))) > "$@.tmp"
	@if cmp -s "$@.tmp" "$@"; then rm "$@.tmp"; else mv "$@.tmp" "$@"; fi

$(generated) &: $(entry) $(cod_sources) $(seed) $(out)/seed.inputs Makefile | $(out)
	@set -eu; stage=$$(mktemp -d "$(out)/.seed.XXXXXX"); \
		trap 'rm -rf "$$stage"' EXIT; trap 'exit 1' HUP INT TERM; \
		"$(seed)" $(COD_SEED_FLAGS) --no-cache --check "$(entry)" \
			--emit-c-provider "$$stage" --emit-llvm "$$stage/driver.ll" --emit-runtime "$$stage/runtime.cc"; \
		for file in $(generated_names); do test -f "$$stage/$$file"; done; \
		for file in $(generated_names); do mv "$$stage/$$file" "$(out)/$$file"; done

c_compile = "$(clang)" $(CPPFLAGS) $(llvm_cflags) -I"$(out)" -I../../include -I../../cod \
	-std=c11 $(CFLAGS) -MMD -MP -c "$<" -o "$@"

$(c_objects): $(out)/cod_providers.h $(clang) $(llvm_config) $(out)/native.inputs Makefile
$(out)/native/%.o: native/%.c | $(out)/native
	$(c_compile)

$(out)/cod_providers.o: $(out)/cod_providers.c
	$(c_compile)

$(out)/driver.o: $(out)/driver.ll $(clang) $(out)/native.inputs Makefile
	"$(clang)" $(IRFLAGS) -c "$<" -o "$@"

$(out)/runtime.o: $(out)/runtime.cc $(clangxx) $(out)/native.inputs Makefile
	"$(clangxx)" $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c "$<" -o "$@"

$(out)/cod-booter: $(objects) $(llvm_libfiles) $(clangxx) $(out)/native.inputs Makefile
	"$(clangxx)" $(LDFLAGS) $(objects) "-Wl,-rpath,$(llvm_libdir)" $(llvm_ldflags) $(LDLIBS) -o "$@"

-include $(c_objects:.o=.d) $(out)/runtime.d

ifndef  common_src_makes_defs_make
include $(TOP_DIR)/defs.make
endif



#+ to shortcut dependency chain checking on source files
#$(SRCS):;
Makefile:;
%.make:;
%.mk:;
#- to shortcut dependency chain checking on source files


# directory creating rule
%/.dir:
	@$(MKDIR_P) $(dir $@)


etags: TAGS

TAGS: $(SRCS) $(HDRS)
	etags $^

obj objs: $(OBJS)

lib libs:



pre_subdirs:
ifdef PRE_SUBDIRS
	@for d in $(PRE_SUBDIRS); do \
		if test -d $$d; then $(MAKE) -C $$d $(MAKECMDGOALS); fi; \
	done
endif

post_subdirs:
ifdef POST_SUBDIRS
	@for d in $(POST_SUBDIRS); do \
		if test -d $$d; then $(MAKE) -C $$d $(MAKECMDGOALS); fi; \
	done
endif



ifneq "$(filter LIB SHARED_LIB,$(BUILD_TYPE))" ""

all: pre_subdirs lib post_subdirs

lib libs: $(lib_file)

$(lib_file): $(OBJS) $(DEP_FILES)

CLEAN_FILES += lib$(LIB_NAME)

all lib:

endif # BUILD_TYPE == LIB | SHARED_LIB



ifeq "$(BUILD_TYPE)" "LIB"

$(lib_file):
	@echo "> Making library $@"; \
		$(AR) rcsu $@ $(OBJS)
	@$(LN_S) -f $@ lib$(LIB_NAME)

endif	# BUILD_TYPE == LIB


ifeq "$(BUILD_TYPE)" "SHARED_LIB"

$(lib_file):
	@echo "> Linking    $@"; \
		$(CC) -o $@ $(OBJS) $(_LDFLAGS) $(LDFLAGS) $(LDFLAGS_)
	@$(LN_S) -f $@ lib$(LIB_NAME)

endif	# BUILD_TYPE == SHARED_LIB



ifeq "$(BUILD_TYPE)" "SINGLE_SRC_BINS"

$(patsubst %.c.o,%,$(filter %.c.o,$(OBJS))): %: %.c.o $(DEP_FILES)
	@echo "> Linking    $@"; \
		$(CC) -o $@ $< $(_LDFLAGS) $(LDFLAGS) $(LDFLAGS_)

$(patsubst %.cc.o,%,$(filter %.cc.o,$(OBJS))): %: %.cc.o $(DEP_FILES)
	@echo "> Linking    $@"; \
		$(CXX) -o $@ $< $(_LDFLAGS) $(LDFLAGS) $(LDFLAGS_)

all:	pre_subdirs $(BIN_NAMES) post_subdirs

  INST_BIN_FILES ?= $(BIN_NAMES)

endif	# BUILD_TYPE == SINGLE_SRC_BINS


ifeq "$(BUILD_TYPE)" "MULTI_SRC_BIN"

 ifneq "$(filter %.c.o %.cc.o,$(OBJS))" ""
  _ld_ = $(CC)
 endif

all:	pre_subdirs $(BIN_NAME) post_subdirs

$(BIN_NAME)  : $(OBJS) $(DEP_FILES)
	@echo "> Linking    $@"; \
		$(_ld_) -o $@ $(OBJS) $(_LDFLAGS) $(LDFLAGS) $(LDFLAGS_)

  INST_BIN_FILES ?= $(BIN_NAME)

endif	# BUILD_TYPE == MULTI_SRC_BIN



ifeq "$(filter LKMOD,$(BUILD_TYPE))" "LKMOD"

lkmodld_flags  = -Ur --sort-common --warn-common #--warn-section-align
lkmodld_flags += $(_LDFLAGS) $(LDFLAGS) $(LDFLAGS_)

all: pre_subdirs $(OBJS) lkmod post_subdirs

lkmod: $(lkmod_file)

$(lkmod_file): $(OBJS) $(DEP_FILES)
	@echo "> Linking    $@"; \
	$(LD) $(lkmodld_flags) -o $@ \
		$(filter %begin.c.o,$(OBJS)) \
		$(filter-out %begin.c.o %end.c.o $(lkmod_file),$(OBJS)) \
		$(filter   %end.c.o,$(OBJS))

# 2.1.18 == 131346
%.ver: %.c $(DEP_FILES)
	@echo "> Generating $@"; \
	$(CPP) $< $(all_cpp_flags) -D__GENKSYMS__ | \
	if test `expr $(lkversion_major) \* 65536 + $(lkversion_minor) \* 256 + $(lkversion_revision)` -lt 131346; then \
		$(GENKSYMS) $(genksyms_smp_prefix) -k $(lkversion_base) . && $(MV) $(notdir $@) $(notdir $@).ver && $(MV) $(notdir $@).ver $@; \
	else \
		$(GENKSYMS) $(genksyms_smp_prefix) -k $(lkversion_base) >$@; \
	fi

endif	# BUILD_TYPE == LKMOD


%.c.o:	%.c $(DEP_FILES)
	@echo "> Compiling  $<"; \
		$(CC) -c $< -o $@ $(_CFLAGS) $(CFLAGS) $(CFLAGS_)

%.cc.o:	%.cc $(DEP_FILES)
	@echo "> Compiling  $<"; \
		$(CXX) -c $< -o $@ $(_CFLAGS) $(CFLAGS) $(CFLAGS_)

%.c.lo:	%.c $(DEP_FILES)
	@echo "> Compiling  $<"; \
		$(CC) -c $< -o $@ $(_CFLAGS) $(CFLAGS) $(CFLAGS_)

%.cc.lo: %.cc $(DEP_FILES)
	@echo "> Compiling  $<"; \
		$(CXX) -c $< -o $@ $(_CFLAGS) $(CFLAGS) $(CFLAGS_)


#+ dependencies generation

deps = $(addsuffix .dep,$(SRCS))

dep_gen_cmd_tail = -M $(_CFLAGS) $(CFLAGS) $(CFLAGS_) -w $< > $@.tmp.dep \
	&& sed '\''s!$(subst .,\.,$*.o) *:!$(patsubst %.dep,%.o,$@) $(patsubst %.dep,%.lo,$@) $@ $(patsubst %.dep,%.s,$@):!g'\'' \
	   < $@.tmp.dep > $@.tmp2.dep \
	&& $(MV) $@.tmp2.dep $@ && $(RM) $@.tmp.dep; } \
	|| $(RM) $@ $@.tmp.dep $@.tmp.dep

  c_deps_gen_cmd = $(SHELL) -ec '{ $(CC)  -D__GENDEPS__ $(dep_gen_cmd_tail)'


%.c.dep: %.c $(DEP_FILES)
	@echo "> Generating dependencies for $<"; \
		$(c_deps_gen_cmd)

%.cc.dep: %.cc $(DEP_FILES)
	@echo "> Generating dependencies for $<"; \
		$(c_deps_gen_cmd)

#- dependencies generation


clean: pre_subdirs post_subdirs
	@echo "> Cleaning up"
	-@$(RM) $(CLEAN_FILES) $(BIN_NAMES) $(BIN_NAME) $(lib_file) \
		*.dep *.ver *.o *.lo *.s *.bak *.core tags *~

distclean: clean pre_subdirs post_subdirs
	@$(RM) .cvsignore




all: pre_subdirs post_subdirs


_depclean:
	@$(RM) *.dep

depclean depsclean: pre_subdirs post_subdirs
	-@echo "> Cleaning up dependencies"; \
		$(RM) *.dep

depend deps dep: pre_subdirs $(deps) post_subdirs


ifeq "$(deps)" ""
  DONT_INCLUDE_DEPS = defined
endif

ifneq "$(filter %clean %_uninstall doc_% include_% dist% none,$(MAKECMDGOALS))" ""
  DONT_INCLUDE_DEPS = defined
endif

ifndef DONT_INCLUDE_DEPS
  -include $(deps)
endif

# common Makefile defs
common_src_makes_defs_make = included



# Don't define any builtin rules and variables.
MAKEFLAGS := $(MAKEFLAGS)R

# Delete default suffixes
.SUFFIXES:

# Delete default rules
.DEFAULT:

.DEFAULT:
	$(error no rules for target $@)

# Tell GNU make 3.79 not to run the top level in parallel
.NOTPARALLEL:


-include local.mk


TOP_DIR     ?= ..
#TOP_INC_DIR ?= $(TOP_DIR)/include
#TOP_LIB_DIR ?= $(TOP_DIR)/lib

ABS_CUR_DIR := $(realpath .)
TOP_CUR_DIR := $(notdir $(ABS_CUR_DIR))

SRC_DIR ?= $(ABS_CUR_DIR)
BLD_DIR ?= $(ABS_CUR_DIR)

VPATH ?= $(BLD_DIR) $(SRC_DIR)


CC     ?= $(CROSS_PREFIX)gcc
CXX    ?= $(CROSS_PREFIX)g++
LD     ?= $(CROSS_PREFIX)ld
AR     ?= $(CROSS_PREFIX)ar
RANLIB ?= $(CROSS_PREFIX)ranlib

RM     = rm -f
RM_R   = $(RM) -r
MV     = mv
LN     = ln
LN_S   = $(LN) -s

MKDIR   = mkdir
MKDIR_P = $(MKDIR) -p
RMDIR   = rmdir --ignore-fail-on-non-empty
RMDIR_P = $(RMDIR) -p


# FLAGS sequence:
#  _CFLAGS  CFLAGS  CFLAGS_
# _LDFLAGS LDFLAGS LDFLAGS_
#
# CFLAGS, LDFLAGS reserved to supply flags from outside
# append _CFLAGS, _LDFLAGS in Makefiles
# use CFLAGS_, LDFLAGS_ rarely to override some flags from CFLAGS, LDFLAGS

# use CFLAGS, LDFLAGS to supply flags from outside
#CFLAGS=

# use CFLAGS_, LDFLAGS_ to override CFLAGS, LDFLAGS


# default _CFLAGS. Don't overwrite it, append gcc options instead
_CFLAGS += -Os -pipe -I$(SRC_DIR)/include -I$(SRC_DIR)

# default _LDFLAGS. Don't overwrite it, append ld options instead
_LDFLAGS += $(LIBS:%=-l%)

ifneq "$(strip $(XWLIBS))" ""
XWLIB_DIRS     = $(addprefix $(TOP_DIR)/,$(XWLIBS))
XWLIB_INC_DIRS = $(addsuffix /include,$(XWLIB_DIRS))
 _CFLAGS += $(addprefix -I,$(XWLIB_INC_DIRS))
_LDFLAGS += $(addprefix -L,$(XWLIB_DIRS)) $(XWLIBS:%=-l%)
endif


ifndef NOSTRICT
STRICT = defined
endif

ifdef STRICT
 _CFLAGS += -Wall -W
 _CFLAGS += -Wpointer-arith
 _CFLAGS += -Wcast-qual
 _CFLAGS += -Wcast-align
 _CFLAGS += -Wwrite-strings
 _CFLAGS += -Winline
 _CFLAGS += -Wbad-function-cast
 _CFLAGS += -Wstrict-prototypes
 _CFLAGS += -Werror-implicit-function-declaration
 _CFLAGS += -fno-common
else
 _CFLAGS += -w	# supress warnings
endif	# STRICT


all_wildcards := $(notdir $(wildcard $(SRC_DIR)/*))
c_srcs := $(filter %.c, $(all_wildcards))
c_hdrs := $(filter %.h, $(all_wildcards))

cc_srcs := $(filter %.cc %.cpp, $(all_wildcards))

SRCS ?= $(c_srcs) $(cc_srcs)
HDRS ?= $(c_hdrs)
INST_HDRS ?= $(HDRS)

ifeq "$(BUILD_TYPE)" "SHARED_LIB"
  OBJS ?= $(patsubst %,%.lo,$(SRCS))
else
  OBJS ?= $(patsubst %,%.o,$(SRCS))
endif


DEP_FILES += $(MAKEFILE_LIST)


ifneq "$(filter LIB SHARED_LIB,$(BUILD_TYPE))" ""
  ifeq  "$(LIB_NAME)" ""
    $(error LIB_NAME unspecified)
  endif
endif

ifeq "$(BUILD_TYPE)" "LIB"
  lib_file := lib$(LIB_NAME).a
endif

ifeq "$(BUILD_TYPE)" "SHARED_LIB"
  lib_file := lib$(LIB_NAME).so

   _CFLAGS += -fPIC
  _LDFLAGS += -shared -Wl,-soname,lib$(LIB_NAME).so
endif


ifeq "$(BUILD_TYPE)" "SINGLE_SRC_BINS"
  BIN_NAMES ?= $(basename $(SRCS))
endif


ifeq "$(filter LKMOD,$(BUILD_TYPE))" "LKMOD"
  ifeq  "$(LKMOD_NAME)" ""
    $(error LKMOD_NAME unspecified)
  endif

  lkmod_suffix = .o
 #lkmod_suffix = .ko
 #lkmod_suffix =

  lkmod_file = $(LKMOD_NAME)$(lkmod_suffix)

  CLEAN_FILES += $(lkmod_file)

  _CFLAGS += -D__KERNEL__ -DMODULE
  _CFLAGS += -fno-strict-aliasing
  _CFLAGS += -isystem $(LK_INC_DIR)
  _CFLAGS += -Wno-unused

ifndef STRICT
  _CFLAGS += -Wno-bad-function-cast
  _CFLAGS += -Wno-sign-compare -Wno-cast-align
endif

  #INC_DIRS  += $(gcc_inc_dir)
  #CPP_FLAGS += -nostdinc

  #lkversion            = $(dfconfig_time_lkversion)
  #lkversion_base       = $(dfconfig_time_lkversion_base)
  #lkversion_major      = $(dfconfig_time_lkversion_major)
  #lkversion_minor      = $(dfconfig_time_lkversion_minor)
  #lkversion_revision   = $(dfconfig_time_lkversion_revision)
  #lkversion_extra      = $(dfconfig_time_lkversion_extra)

  #ifdef lk_config_modversions
  #  CPP_FLAGS += --include=$(TOP_DIR)/linux/df/modversions.h
  #endif
endif	# BUILD_TYPE == LKMOD


ifdef STRICT
  ALL_CFLAGS = $(filter-out -w,$(_CFLAGS) $(CFLAGS) $(CFLAGS_))
else
  ALL_CFLAGS =                 $(_CFLAGS) $(CFLAGS) $(CFLAGS_)
endif

ALL_LDFLAGS = $(_LDFLAGS) $(LDFLAGS) $(LDFLAGS_)


.PHONY: all clean distclean install uninstall etags
.PHONY: pre_subdirs post_subdirs
all:

bin_PROGRAMS = svd

svd_SOURCES = \
svd_cfg.c \
svd_ua.c \
svd_atab.c \
svd.c 

#INCLUDES = $(SOFIA_SIP_UA_CFLAGS) -Wunused
#svd_LDADD = $(SOFIA_SIP_UA_LIBS) -lpthread -L. -L./libab/ -lconfig -lab

INCLUDES = -Wunused -I../../libab/
svd_LDADD = -lpthread -L. -L../../libab/ -lsofia-sip-ua -lconfig -lab
svd_LDFLAGS = -Wl,-S -static


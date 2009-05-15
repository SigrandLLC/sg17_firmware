bin_PROGRAMS = svd

svd_SOURCES = \
svd_cfg.c \
svd_ua.c \
svd_atab.c \
svd.c 

INCLUDES = -Wunused -I../../libab/ -I../../libconfig/
svd_LDADD = -lpthread -L. -L../../libab/ -L../../libconfig/ -lsofia-sip-ua -lconfig -lab
svd_LDFLAGS = -Wl,-S -static


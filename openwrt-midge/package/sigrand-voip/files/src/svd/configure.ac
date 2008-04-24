AC_INIT(svd, 0.1, vlad.luch@mail.ru)
AC_CONFIG_SRCDIR([src/svd.c])
AC_CONFIG_HEADER([config.h])

AC_DEFINE(DFS, 
	[for(g_f_cnt = 0; g_f_cnt < g_f_offset; g_f_cnt++)					SU_DEBUG_9(("  "));							SU_DEBUG_9(("vvvv %s() vvvv\n", __func__));					g_f_offset++;], 
	[Debug Function Start marker])

AC_DEFINE(DFE, 
	[g_f_offset--;									for(g_f_cnt = 0; g_f_cnt < g_f_offset; g_f_cnt++)					SU_DEBUG_9(("  "));							SU_DEBUG_9(("^^^^ %s() ^^^^\n", __func__));], 
	[Debug Function End marker])

AM_INIT_AUTOMAKE

# Checks for programs.

AC_PROG_CC

# Checks for libraries.
#PKG_CHECK_MODULES(SOFIA_SIP_UA, sofia-sip-ua >= 1.11.6)
#AC_SUBST(SOFIA_SIP_UA_LIBS)
#AC_SUBST(SOFIA_SIP_UA_CFLAGS)
#AC_SUBST(SOFIA_SIP_UA_VERSION)

# Create files

AC_CONFIG_FILES([
Makefile
doc/Makefile
src/Makefile
])

AC_OUTPUT

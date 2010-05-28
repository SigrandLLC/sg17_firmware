if test -n "${CCACHE}${_CCACHE_PATH_}"; then
   echo 1>&2 "ccache was set before"
fi

ccache_full=`which ccache 2>/dev/null`
if test -n "$ccache_full"; then
   export CCACHE=$ccache_full
   ccache_bin=`dirname $ccache_full`
   ccache_lib=$ccache_bin/../lib/ccache
   export _CCACHE_PATH_=$ccache_lib
   OLD_PATH=$PATH
   PATH=$_CCACHE_PATH_:$PATH
fi

unset ccache_full ccache_bin ccache_lib


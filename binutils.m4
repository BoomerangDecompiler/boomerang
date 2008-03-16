dnl $Id$

AC_DEFUN(AC_PATH_BINUTIL, [
   for i in $2; do
      if test -f ${TARGET_PREFIX}$i; then
          AC_MSG_CHECKING([for ${TARGET_PREFIX}$i])
          $1=${TARGET_PREFIX}$i
	  AC_MSG_RESULT( [yes] )
      else
         AC_PATH_PROG( $1, ${TARGET_PREFIX}$i )
      fi
      if test -n "$$1"; then
         break
      fi
   done
   if test -z "$$1"; then
      # not found, try to fall back on basic versions
      AC_PATH_PROGS( $1, $2 )
   fi
])

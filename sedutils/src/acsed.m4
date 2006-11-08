AC_DEFUN(
   SED_LIB_GLIB,[
   AC_CACHE_CHECK(
      [for libglib >= $1.$2.$3],
      sed_cv_lib_glib,
      [AC_TRY_RUN(
         [#include "glib.h"
         int main(void) { exit(!GLIB_CHECK_VERSION($1,$2,$3) ); }],
         sed_cv_lib_glib=yes,
         sed_cv_lib_glib=no,
         sed_cv_lib_glib=yes)])])

AC_DEFUN(
   SED_LIB_SED,[
   AC_CHECK_HEADER(
      sed_sedflux.h,,
      [GLIB_ADD_TO_VAR(CPPFLAGS,"-I${prefix}/include","-I${prefix}/include")
       echo '*** assuming sed_sedflux.h will be created later'
       echo "*** adding ${prefix}/include to \$CPPFLAGS"])
   AC_CACHE_CHECK(
      [for libsed >= $1.$2.$3],
      sed_cv_lib_sed,
      [AC_TRY_RUN(
         [#include "sed_const.h"
         int main(void)
         {
            exit(!S_CHECK_VERSION($1,$2,$3));
         }],
         sed_cv_lib_sed=yes,
         sed_cv_lib_sed=no,
         [echo "cross compiling; assumed ok..."])])
#   if test $sed_cv_lib_sed = no; then
      GLIB_ADD_TO_VAR(LDFLAGS,"-L${prefix}/lib","-L${prefix}/lib")
      echo "*** assuming -lsedlfux will be created later"
      echo "*** adding ${prefix}/lib to \$LDFLAGS"
#   fi
])

dnl GLIB_STR_CONTAINS (SRC_STRING, SUB_STRING [, CONTAINS_ACTION] [, ELSE_ACTION ])
AC_DEFUN(GLIB_STR_CONTAINS,[
        case "[$1]" in
        *"[$2] "*[)]
                [$3]
                ;;
        *[)]
                [$4]
                ;;
        esac
])
dnl GLIB_ADD_TO_VAR (ENV_VARIABLE, CHECK_STRING, ADD_STRING)
AC_DEFUN(GLIB_ADD_TO_VAR,[
        GLIB_STR_CONTAINS($[$1], [$2], [$1]="$[$1]", [$1]="$[$1] [$3]")
])


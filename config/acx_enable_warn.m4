# This function is used to add compiler specific warning flags to CFLAGS and
# CXXFLAGS environment variables. Users are expected to specify their own warning
# flags for unknown compilers or special use cases by adding appropriate values
# to CFLAGS and CXXFLAGS.
AC_DEFUN([ACX_ENABLE_WARN], [
  acx_enable_warn=""
  acx_enable_warn_flags=""
  acx_enable_warn_compiler="$CXXVENDOR"
  
  # Allow the user to enable or disable warnings
  AC_ARG_ENABLE([warning],
    [AC_HELP_STRING([--enable-warning@<:@=yes|no|GNU|clang|Pathscale|Portland|Intel|IBM@:>@],
      [Automatically set warnings for compiler.@<:@default=yes@:>@])],
    [
      case $enableval in
      yes)
        acx_enable_warn="yes"
      ;;
      no)
        acx_enable_warn="no"
      ;;
      *)
        acx_enable_warn="yes"
        acx_enable_warn_compiler="$enableval"
      esac
    ],
    [acx_enable_warn="yes"]
  )
  
  # Automatically specify the warning flags for known compilers. 
  if test $acx_enable_warn != "no"; then
    case $acx_enable_warn_compiler in
      GNU)
        acx_enable_warn_flags="-Wall -Wno-strict-aliasing -Wno-deprecated"
      ;;
      clang)
        acx_enable_warn_flags="-Wall"
      ;;
      Pathscale)
        acx_enable_warn_flags="-Wall"
      ;;
      Portland)
        acx_enable_warn_flags=""
      ;;
      Intel)
        acx_enable_warn_flags="-Wall -diag-disable remark,279,654,1125"
      ;;
      IBM)
        acx_enable_warn_flags=""
      ;;
      *)
        AC_MSG_WARN([Warning flags not set for $acx_enable_optimal_compile compiler])
      ;;
    esac

    # Test the flags and add them to flag variables if successful.
    ACX_CHECK_COMPILER_FLAG([C], [CFLAGS], [$acx_enable_warn_flags],
      [CFLAGS="$CFLAGS $acx_enable_warn_flags"],
      [AC_MSG_WARN([$CC does not accept $acx_enable_warn_flags, no warning flags will be used.])])
    ACX_CHECK_COMPILER_FLAG([C++], [CXXFLAGS], [$acx_enable_warn_flags],
      [CXXFLAGS="$CXXFLAGS $acx_enable_warn_flags"],
      [AC_MSG_WARN([$CXX does not accept $acx_enable_warn_flags, no warning flags will be used.])])
  fi
  

])

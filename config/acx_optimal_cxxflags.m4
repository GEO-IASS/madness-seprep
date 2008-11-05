AC_DEFUN([ACX_OPTIMAL_CXXFLAGS], [
    AC_DETECT_CXX

    save_CXXFLAGS="$CXXFLAGS"
    case $CXXVENDOR in
         GNU)
            # Delete trailing -stuff from X.X.X-stuff then parse
            CXXVERSION=[`$CXX -dumpversion | sed -e 's/-.*//'`]
            CXXMAJOR=[`echo $CXXVERSION | sed -e 's/\.[.0-9a-zA-Z\-_]*//'`]
            CXXMINOR=[`echo $CXXVERSION | sed -e 's/[0-9]*\.//' -e 's/\.[0-9]*//'`]
            CXXMICRO=[`echo $CXXVERSION | sed -e 's/[0-9]*\.[0-9]*\.//'`]
            echo "Setting compiler flags for GNU C++ major=$CXXMAJOR minor=$CXXMINOR micro=$CXXMICRO"

            TARGET_ARCH="native"
            if test "x$HAVE_CRAYXT" = xyes; then
                echo "Setting target architecture for GNU compilers to barcelona for CRAYXT"
                TARGET_ARCH=barcelona
            fi

            # -fomit-frame-pointer 
            CXXFLAGS="-g -Wall -Wno-strict-aliasing -Wno-deprecated -ansi -O3 -ffast-math -mfpmath=sse"
            if test $CXXMAJOR -gt 4; then
               CXXFLAGS="$CXXFLAGS -march=$TARGET_ARCH"
            elif test $CXXMAJOR -eq 4; then
               if test $CXXMINOR -gt 1; then
                  CXXFLAGS="$CXXFLAGS -march=$TARGET_ARCH"
               fi
            fi
            ;;

         Pathscale)
            CXXFLAGS="-Wall -Ofast"
            if test "x$HAVE_CRAYXT" = xyes; then
                echo "Setting Pathscale CXX architecture to -barcelona for Cray-XT"
                CXXFLAGS="$CXXFLAGS -march=barcelona"             
            fi
            ;;

         Portland)
            CXXFLAGS="-O3 -fastsse -Mflushz -Mcache_align"    
            echo "Appending -pgf90libs to LIBS so can link against Fortran BLAS/linalg"
            LIBS="$LIBS -pgf90libs"
            if test "x$HAVE_CRAYXT" = xyes; then
                echo "Setting PGI CXX architecture to -tp barcelona-64 for Cray-XT"
                CXXFLAGS="$CXXFLAGS -tp barcelona-64"             
            fi
            ;;

         Intel)
            CXXFLAGS="-Wall -fast -ansi"
            ;;

         unknown)
            ;;

         *)
            ;;
    esac
    CXXFLAGS="$CXXFLAGS -D_REENTRANT "
    echo "Changing CXXFLAGS from '$save_CXXFLAGS'"
    echo "to '$CXXFLAGS'"
])

AC_DEFUN([ACX_OPTIMAL_CXXFLAGS], [
    AC_DETECT_CXX

    save_CXXFLAGS="$CXXFLAGS"
    case $CXXVENDOR in
         GNU)
            CXXMAJOR=[`$CXX -dumpversion | sed -e 's/\.[.0-9]*//'`]
            CXXMINOR=[`$CXX -dumpversion | sed -e 's/[0-9]*\.//' -e 's/\.[0-9]*//'`]
            CXXMICRO=[`$CXX -dumpversion | sed -e 's/[0-9]*\.[0-9]*\.//'`]
            echo "Setting compiler flags for GNU C++ major=$CXXMAJOR minor=$CXXMINOR micro=$CXXMICRO"


            CXXFLAGS="-Wall -Wno-strict-aliasing -Wno-deprecated -ansi -O3 -ffast-math -fomit-frame-pointer -mfpmath=sse"
            if test $CXXMAJOR -ge 4; then
               CXXFLAGS="$CXXFLAGS -march=native"
            fi
            ;;

         Pathscale)
            CXXFLAGS="-Wall -Ofast"
            ;;

         Portland)
            CXXFLAGS="-fastsse -Mflushz -Mcache_align"    
            ;;

         Intel)
            CXXFLAGS="-Wall -fast -ansi"
            ;;

         unknown)
            ;;

         *)
            ;;
    esac
    echo " Changing CXXFLAGS from '$save_CXXFLAGS'"
    echo " to '$CXXFLAGS'"
])
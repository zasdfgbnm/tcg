# https://github.com/sio2project/sio2jail/blob/master/external/libseccomp.cmake

IF(LINK STREQUAL "STATIC")
    SET(libseccomp_LIB_FILE_NAME "libseccomp.a")
ELSE()
    SET(libseccomp_LIB_FILE_NAME "libseccomp.so")
ENDIF()
FIND_FILE(
    libseccomp_LIB_PATH
    NAMES "${libseccomp_LIB_FILE_NAME}"
    PATHS "${LIBSECCOMP_PREFIX}" "${LIBSECCOMP_PREFIX}/lib" "${LIBSECCOMP_PREFIX}/usr/lib"
    )
FIND_PATH(
    libseccomp_INC_PATH
    NAMES seccomp.h
    PATHS "${LIBSECCOMP_PREFIX}" "${LIBSECCOMP_PREFIX}/usr/include"
    )
IF(libseccomp_LIB_FILE_NAME MATCHES "NOTFOUND")
    MESSAGE("-- Libseccomp not found")
ELSE()
    EXECUTE_PROCESS(
        COMMAND
        bash -c "
            exe=`mktemp`
            echo -e '#include<stdio.h>\n#include<seccomp.h>\nint main(){printf(\"%d.%d\",SCMP_VER_MAJOR,SCMP_VER_MINOR);}' \
                | gcc -I ${libseccomp_INC_PATH} -xc /dev/stdin -o $exe >/dev/null 2>&1 && $exe
            rc=$?
            rm -f $exe
            exit $rc"
        OUTPUT_VARIABLE libseccomp_VERSION
        RESULT_VARIABLE libseccomp_VERSION_RC
        )
    IF(NOT libseccomp_VERSION_RC EQUAL 0 OR libseccomp_VERSION VERSION_LESS 2.3)
        SET(libseccomp_LIB_PATH "libseccomp_LIB_PATH-NOTFOUND")
        SET(libseccomp_INC_PATH "libseccomp_INC_PATH-NOTFOUND")
        IF (NOT libseccomp_VERSION_RC EQUAL 0)
            MESSAGE("-- failed to compile Libseccomp test program")
        ELSE()
            MESSAGE("-- found Libseccomp in version ${libseccomp_VERSION}, but minimal required version is 2.3")
        ENDIF()
    ENDIF()
ENDIF()

IF(libseccomp_LIB_PATH MATCHES NOTFOUND OR libseccomp_INC_PATH MATCHES NOTFOUND)
    MESSAGE("-- Libseccomp not found.")
ELSE()
    MESSAGE("-- Libseccomp configuration:")
    MESSAGE("--   library: ${libseccomp_LIB_PATH}")
    MESSAGE("--   include directory: ${libseccomp_INC_PATH}")

    SET(USE_SECCOMP ON)
    ADD_LIBRARY(seccomp UNKNOWN IMPORTED)
    SET_PROPERTY(TARGET seccomp
        PROPERTY IMPORTED_LOCATION
            "${libseccomp_LIB_PATH}"
        )
    INCLUDE_DIRECTORIES("${libseccomp_INC_PATH}")
ENDIF()
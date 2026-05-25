# Java bindings for libpHash.
#
# Builds pHash.jar from org/phash/*.java and a JNI shared library
# (libpHash-jni) implementing the native methods declared on those classes.
# The JNI header (org_phash_pHash.h) is generated at build time by
# `javac -h` -- it is not checked in.

find_package(Java COMPONENTS Development)
find_package(JNI)
if(NOT Java_FOUND OR NOT JNI_FOUND)
    message(STATUS "Java SDK or JNI headers not found -- skipping Java bindings")
    return()
endif()
include(UseJava)

file(GLOB PHASH_JAVA_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     "${CMAKE_CURRENT_SOURCE_DIR}/org/phash/*.java")

# add_jar with GENERATE_NATIVE_HEADERS drives javac -h and writes the
# JNI headers (one per class with native methods) into the given dir.
set(JNI_HEADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/jni-headers)
file(MAKE_DIRECTORY ${JNI_HEADER_DIR})

add_jar(pHash-jar
    SOURCES ${PHASH_JAVA_SOURCES}
    OUTPUT_NAME pHash
    GENERATE_NATIVE_HEADERS pHash-jni-headers
        DESTINATION ${JNI_HEADER_DIR}
)

add_library(pHash-jni SHARED pHash-jni.cpp)
add_dependencies(pHash-jni pHash-jar)

target_include_directories(pHash-jni PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${JNI_INCLUDE_DIRS}
    ${JNI_HEADER_DIR}
)
target_link_libraries(pHash-jni PRIVATE pHash ${JNI_LIBRARIES})

set_target_properties(pHash-jni PROPERTIES VERSION 4.2.0 SOVERSION 4)

install(TARGETS pHash-jni DESTINATION ${CMAKE_INSTALL_LIBDIR})
install_jar(pHash-jar DESTINATION ${CMAKE_INSTALL_DATADIR}/java)

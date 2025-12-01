cmake_minimum_required(VERSION 3.28)
project (strategy)

message(STATUS "Source directory is " ${CMAKE_SOURCE_DIR})
message(STATUS "Build directory is " ${CMAKE_CURRENT_BINARY_DIR})
message(STATUS "WT_INCLUDE: " ${WT_INCLUDE})
message(STATUS "BOOST_INCLUDE_DIR: " ${BOOST_INCLUDE_DIR})
message(STATUS "BOOST_LIB_DIRS: " ${BOOST_LIB_DIRS})

include_directories(${BOOST_INCLUDE_DIR})
include_directories(${WT_INCLUDE})
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

set(CMAKE_CXX_STANDARD 17)

#//////////////////////////
# Wt web client 
#//////////////////////////

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/wt-4.12.1/src)
include_directories(ext/asio-1.30.2/asio/include)

include_directories(${CMAKE_SOURCE_DIR}/ext/sqlite)

set(SOURCES
    src/main.cpp
    src/app.cc
    src/login.cc
    src/projects.cc
    src/search.cc
    src/library.cc
    src/report.cc
    src/data.cc
    src/ssl_read.cc
    src/get.cc
    src/api.cc
    src/lite.cc
    src/odbc.cc
    src/db_interface.cc
    ext/sqlite/sqlite3.c
)

set(HEADERS
    src/app.hh
    src/login.hh
    src/projects.hh
    src/search.hh
    src/library.hh
    src/report.hh
    src/data.hh
    src/ssl_read.hh
    src/get.hh
    src/api.hh
    src/lite.hh
    src/odbc.hh
    src/db_interface.hh
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

if (MSVC)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
  set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "--http-address=0.0.0.0 --http-port=8080  --docroot=.")
  set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()

#//////////////////////////
# Linux/Mac
#//////////////////////////

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations -pthread")
endif()

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-deprecated-copy -Wno-deprecated-copy-dtor")
endif()

message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")

#//////////////////////////
# link with libraries
# lib_dep contains a cascade definition of all the libraries needed to link
#//////////////////////////

set(lib_dep ${lib_dep})

#//////////////////////////
# OpenSSL
#//////////////////////////

if (MSVC)
  if (OPEN_SSL_SOURCE)
    set(OPENSSL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/openssl-3.0.5)
    set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
    set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/libcrypto.lib)
    set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/libssl.lib)
  else()
    set(OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT_DIR})
    message(STATUS "OpenSSL root folder from environment, (OPENSSL_ROOT_DIR): ${OPENSSL_ROOT_DIR}")
  endif()
endif()

find_package(OpenSSL REQUIRED)

message(STATUS "OpenSSL root: ${OPENSSL_ROOT_DIR}")
message(STATUS "OpenSSL include: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL libs: ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}")

if(UNIX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

include_directories(${OPENSSL_INCLUDE_DIR})

set(lib_dep ${lib_dep} ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
if (MSVC)
  set(lib_dep ${lib_dep} crypt32.lib ws2_32.lib wsock32.lib odbc32.lib)
endif()

#//////////////////////////
# LINUX
# order of the link libraries matters; pthread dl
#//////////////////////////

if(LINUX)
  set(lib_dep ${lib_dep} pthread dl odbc)
  set(lib_dep ${lib_dep} stdc++fs) 
  find_program(LSB_RELEASE_EXEC lsb_release)
  execute_process(COMMAND ${LSB_RELEASE_EXEC} -is OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Building in " ${LSB_RELEASE_ID_SHORT})
endif()

if (MSVC)
  set(lib_dep ${lib_dep} ${CMAKE_CURRENT_SOURCE_DIR}/build/wt-4.12.1/src/Debug/wtd.lib)
  set(lib_dep ${lib_dep} ${CMAKE_CURRENT_SOURCE_DIR}/build/wt-4.12.1/src/http/Debug/wthttpd.lib)
  set(lib_dep ${lib_dep} ${BOOST_LIB_DIRS}/libboost_filesystem-vc143-mt-gd-x64-1_88.lib)
  set(lib_dep ${lib_dep} ${BOOST_LIB_DIRS}/libboost_thread-vc143-mt-gd-x64-1_88.lib)
  set(lib_dep ${lib_dep} ${BOOST_LIB_DIRS}/libboost_program_options-vc143-mt-gd-x64-1_88.lib)
  set(lib_dep ${lib_dep} ${BOOST_LIB_DIRS}/libboost_chrono-vc143-mt-gd-x64-1_88.lib)
endif()

if(APPLE)
  set(lib_dep ${lib_dep} ${CMAKE_CURRENT_SOURCE_DIR}/install/wt/lib/libwt.dylib)
  set(lib_dep ${lib_dep} ${CMAKE_CURRENT_SOURCE_DIR}/install/wt/lib/libwthttp.dylib)
  set(lib_dep ${lib_dep} ${BOOST_LIB_DIRS}/libboost_filesystem-clang-darwin17-mt-x64-1_88.a)
  set(lib_dep ${lib_dep} ${BOOST_LIB_DIRS}/libboost_thread-clang-darwin17-mt-x64-1_88.a)
  set(lib_dep ${lib_dep} ${BOOST_LIB_DIRS}/libboost_program_options-clang-darwin17-mt-x64-1_88.a)
  set(lib_dep ${lib_dep} odbc)
endif()

if (LINUX)
  set(lib_dep ${lib_dep} ${CMAKE_CURRENT_SOURCE_DIR}/install/wt/lib/libwt.so)
  set(lib_dep ${lib_dep} ${CMAKE_CURRENT_SOURCE_DIR}/install/wt/lib/libwthttp.so)
  set(lib_dep ${lib_dep} ${BOOST_LIB_DIRS}/libboost_filesystem-gcc13-mt-x64-1_88.a)
  set(lib_dep ${lib_dep} ${BOOST_LIB_DIRS}/libboost_thread-gcc13-mt-x64-1_88.a)
  set(lib_dep ${lib_dep} ${BOOST_LIB_DIRS}/libboost_program_options-gcc13-mt-x64-1_88.a)
endif()

#//////////////////////////
#  link
#//////////////////////////

message(STATUS "lib_dep: " ${lib_dep})
target_link_libraries(${PROJECT_NAME} ${lib_dep})

cmake_minimum_required (VERSION 2.8)
project (kbpm)
aux_source_directory(../lib DIR_LIBS)
aux_source_directory(../src DIR_SRCS)
add_executable(kbpm ${DIR_LIBS} ${DIR_SRCS})

# -lpthread
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  set_property(TARGET kbpm PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET kbpm PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(kbpm "${CMAKE_THREAD_LIBS_INIT}")
endif()

# -lm
find_library(M_LIB m)
target_link_libraries(kbpm ${M_LIB})

install(PROGRAMS kbpm  DESTINATION /usr/local/bin)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kalen Blue")
set(CPACK_PACKAGE_VERSION "0.1.0")
 
include(CPack)
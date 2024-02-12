# Install script for directory: C:/Users/HP ELiteBook/Documents/Year 3/COMP30030 Third Year Project Laboratory/2. Code/skeletonMaker/dependencies/fltk-1.3.9/jpeg

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/FLTK")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/HP ELiteBook/Documents/Year 3/COMP30030 Third Year Project Laboratory/2. Code/skeletonMaker/dependencies/fltk-1.3.9/build/lib/Debug/fltk_jpegd.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/HP ELiteBook/Documents/Year 3/COMP30030 Third Year Project Laboratory/2. Code/skeletonMaker/dependencies/fltk-1.3.9/build/lib/Release/fltk_jpeg.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/HP ELiteBook/Documents/Year 3/COMP30030 Third Year Project Laboratory/2. Code/skeletonMaker/dependencies/fltk-1.3.9/build/lib/MinSizeRel/fltk_jpeg.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/HP ELiteBook/Documents/Year 3/COMP30030 Third Year Project Laboratory/2. Code/skeletonMaker/dependencies/fltk-1.3.9/build/lib/RelWithDebInfo/fltk_jpeg.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/FL/images" TYPE FILE FILES
    "C:/Users/HP ELiteBook/Documents/Year 3/COMP30030 Third Year Project Laboratory/2. Code/skeletonMaker/dependencies/fltk-1.3.9/jpeg/jconfig.h"
    "C:/Users/HP ELiteBook/Documents/Year 3/COMP30030 Third Year Project Laboratory/2. Code/skeletonMaker/dependencies/fltk-1.3.9/jpeg/jerror.h"
    "C:/Users/HP ELiteBook/Documents/Year 3/COMP30030 Third Year Project Laboratory/2. Code/skeletonMaker/dependencies/fltk-1.3.9/jpeg/jmorecfg.h"
    "C:/Users/HP ELiteBook/Documents/Year 3/COMP30030 Third Year Project Laboratory/2. Code/skeletonMaker/dependencies/fltk-1.3.9/jpeg/jpeglib.h"
    "C:/Users/HP ELiteBook/Documents/Year 3/COMP30030 Third Year Project Laboratory/2. Code/skeletonMaker/dependencies/fltk-1.3.9/jpeg/fltk_jpeg_prefix.h"
    )
endif()


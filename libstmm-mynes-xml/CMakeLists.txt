# File: libstmm-mynes-xml/CMakeLists.txt

#  Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public
#  License along with this program; if not, see <http://www.gnu.org/licenses/>

cmake_minimum_required(VERSION 3.0)

project(stmm-mynes-xml CXX)

set(LIBRARY_OUTPUT_DIRECTORY "build")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../share/cmake")

include(CommonUtil)

CheckBinaryNotSourceTree()
CheckBuildType()
DefineSharedLibOption()
DefineCommonOptions()
DefineCommonCompileOptions("c++14")

# Headers dir
set(STMMI_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/include")
set(STMMI_HEADERS_DIR  "${STMMI_INCLUDE_DIR}/stmm-mynes-xml")
# Header files
set(STMMI_HEADERS
        "${STMMI_HEADERS_DIR}/stmm-mynes-xml-config.h"
        "${STMMI_HEADERS_DIR}/xmlfillerevent.h"
        "${STMMI_HEADERS_DIR}/xmlmynesevent.h"
        "${STMMI_HEADERS_DIR}/xmlpressevent.h"
        "${STMMI_HEADERS_DIR}/xmlsonarevent.h"
        "${STMMI_HEADERS_DIR}/xmlsquarsorevent.h"
        )
#
# Sources dir
set(STMMI_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/src")
# Source files (and headers only used for building)
set(STMMI_SOURCES
        "${STMMI_SOURCES_DIR}/xmlfillerevent.cc"
        "${STMMI_SOURCES_DIR}/xmlmynesevent.cc"
        "${STMMI_SOURCES_DIR}/xmlpressevent.cc"
        "${STMMI_SOURCES_DIR}/xmlsonarevent.cc"
        "${STMMI_SOURCES_DIR}/xmlsquarsorevent.cc"
        )

# Data dir
set(STMMI_DATA_DIR  "${PROJECT_SOURCE_DIR}/data")
# Data files
set(STMMI_DATA_FILES
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/theme.xml"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/thumbnail.png"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/cell-backg.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/cell-off.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/cell-on.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/danger-backg.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/dir-r.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/dir-ru.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/dir-u.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/dir-lu.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/dir-l.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/dir-ld.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/dir-d.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/dir-rd.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/flag-maybe.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/flag-wrong.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/flag.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/flashing.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/mine-bang.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/mine.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/squarehide0.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/squarehide1.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/squarehide2.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/squarehide3.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/squarehide4.svg"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/images/uncover-area.svg"
        #
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/sounds/sonarstart.mp3"
        "${STMMI_DATA_DIR}/themes/mynes/simple_m.thm/sounds/sonarthrow.mp3"
        )

# Define library
add_library(stmm-mynes-xml ${STMMI_SOURCES} "${PROJECT_BINARY_DIR}/stmm-mynes-xml-config.cc")

include("stmm-mynes-xml-defs.cmake")

target_include_directories(stmm-mynes-xml        PUBLIC "${STMMMYNESXML_EXTRA_INCLUDE_DIRS}")
target_include_directories(stmm-mynes-xml SYSTEM PUBLIC "${STMMMYNESXML_EXTRA_INCLUDE_SDIRS}")
target_include_directories(stmm-mynes-xml BEFORE PUBLIC
  $<BUILD_INTERFACE:${STMMI_HEADERS_DIR}>
  $<INSTALL_INTERFACE:include/stmm-mynes-xml>  # <prefix>/include/stmm-mynes-xml
)

target_link_libraries(stmm-mynes-xml ${STMMMYNESXML_EXTRA_LIBRARIES})

set_target_properties(stmm-mynes-xml PROPERTIES ${CMAKE_BUILD_TYPE}_POSTFIX "")
set_target_properties(stmm-mynes-xml PROPERTIES
        VERSION ${STMM_MYNES_XML_VERSION}
        SOVERSION ${STMM_MYNES_XML_MAJOR_VERSION}
#        OUTPUT_NAME "stmm-mynes-xml-${STMM_MYNES_XML_VERSION}"
        )

DefineTargetPublicCompileOptions(stmm-mynes-xml)

# Set version for stmm-mynes-xml-config.cc.in
set(STMMI_PKG_VERSION "${STMM_MYNES_XML_VERSION}")
include(GNUInstallDirs)
set(STMMI_PKG_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/stmm-games")

# Create config file for library
configure_file("${PROJECT_SOURCE_DIR}/src/stmm-mynes-xml-config.cc.in"
               "${PROJECT_BINARY_DIR}/stmm-mynes-xml-config.cc" @ONLY)

# Set install dir for pkg-config file
set(STMMI_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
# Create pkg-config file for the library
configure_file("${PROJECT_SOURCE_DIR}/stmm-mynes-xml.pc.in"
               "${PROJECT_BINARY_DIR}/stmm-mynes-xml.pc" @ONLY)

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "stmm-mynes-xml was configured with the following options:")
message(STATUS " STMMI_HEADERS:                    ${STMMI_HEADERS}")
message(STATUS " STMMI_SOURCES:                    ${STMMI_SOURCES}")
message(STATUS " STMMMYNESXML_EXTRA_INCLUDE_DIRS:  ${STMMMYNESXML_EXTRA_INCLUDE_DIRS}")
message(STATUS " STMMMYNESXML_EXTRA_INCLUDE_SDIRS: ${STMMMYNESXML_EXTRA_INCLUDE_SDIRS}")
message(STATUS " STMMMYNESXML_EXTRA_LIBRARIES:     ${STMMMYNESXML_EXTRA_LIBRARIES}")
message(STATUS " CMAKE_BUILD_TYPE:                 ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:            ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:       ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " CMAKE_CXX_FLAGS:                  ${CMAKE_CXX_FLAGS}")
message(STATUS " install prefix:                   ${STMMI_INSTALL_PREFIX}")
message(STATUS " BUILD_DOCS:                       ${BUILD_DOCS}")
message(STATUS " ENV{STMM_DEBIAN_PACKAGING}:       $ENV{STMM_DEBIAN_PACKAGING}")
message(STATUS " BUILD_TESTING:                    ${BUILD_TESTING}")
message(STATUS " STMMI_PKG_DATA_DIR:               ${STMMI_PKG_DATA_DIR}")
endif()

# Documentation
add_subdirectory(doc)
if (BUILD_DOCS)
    # build automatically, otherwise call `make doc`
    add_dependencies(stmm-mynes-xml doc)
endif()

# Testing
enable_testing()
add_subdirectory(test)

install(TARGETS stmm-mynes-xml LIBRARY DESTINATION "lib"  ARCHIVE DESTINATION "lib")

install(FILES ${STMMI_HEADERS} DESTINATION "include/stmm-mynes-xml")

install(FILES  "${PROJECT_BINARY_DIR}/stmm-mynes-xml.pc"   DESTINATION "lib/pkgconfig")

foreach (STMMI_DATA_CUR_FILE  ${STMMI_DATA_FILES})
    file(RELATIVE_PATH  STMMI_DATA_CUR_REL_FILE  "${STMMI_DATA_DIR}"  "${STMMI_DATA_CUR_FILE}")
    get_filename_component(STMMI_DATA_CUR_DIR "${STMMI_PKG_DATA_DIR}/${STMMI_DATA_CUR_REL_FILE}" DIRECTORY)
    #message(STATUS "STMMI_DATA_CUR_FILE         ${STMMI_DATA_CUR_FILE}")
    #message(STATUS "STMMI_DATA_CUR_REL_FILE     ${STMMI_DATA_CUR_REL_FILE}")
    #message(STATUS "STMMI_DATA_CUR_DIR          ${STMMI_DATA_CUR_DIR}")
    install(FILES ${STMMI_DATA_CUR_FILE}  DESTINATION   "${STMMI_DATA_CUR_DIR}")
endforeach()

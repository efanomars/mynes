# File: libstmm-mynes/CMakeLists.txt

#  Copyright © 2019  Stefano Marsili, <stemars@gmx.ch>
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

project(stmm-mynes CXX)

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
set(STMMI_HEADERS_DIR  "${STMMI_INCLUDE_DIR}/stmm-mynes")
# Header files
set(STMMI_HEADERS
        "${STMMI_HEADERS_DIR}/fillerevent.h"
        "${STMMI_HEADERS_DIR}/mynesevent.h"
        "${STMMI_HEADERS_DIR}/pressevent.h"
        "${STMMI_HEADERS_DIR}/sonarblock.h"
        "${STMMI_HEADERS_DIR}/sonarevent.h"
        "${STMMI_HEADERS_DIR}/squarsorevent.h"
        "${STMMI_HEADERS_DIR}/stmm-mynes-config.h"
        )
#
# Sources dir
set(STMMI_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/src")
# Source files (and headers only used for building)
set(STMMI_SOURCES
            #"${STMMI_SOURCES_DIR}/sonarevent.cc"
        #
        "${STMMI_SOURCES_DIR}/fillerevent.cc"
        "${STMMI_SOURCES_DIR}/mynesevent.cc"
        "${STMMI_SOURCES_DIR}/pressevent.cc"
        "${STMMI_SOURCES_DIR}/sonarblock.cc"
        "${STMMI_SOURCES_DIR}/sonarevent.cc"
        "${STMMI_SOURCES_DIR}/squarsorevent.cc"
        )

# Define library
add_library(stmm-mynes ${STMMI_SOURCES} "${PROJECT_BINARY_DIR}/stmm-mynes-config.cc")

include("stmm-mynes-defs.cmake")

target_include_directories(stmm-mynes        PUBLIC ${STMMMYNES_EXTRA_INCLUDE_DIRS})
target_include_directories(stmm-mynes SYSTEM PUBLIC ${STMMMYNES_EXTRA_INCLUDE_SDIRS})
target_include_directories(stmm-mynes BEFORE PUBLIC
  $<BUILD_INTERFACE:${STMMI_HEADERS_DIR}>
  $<INSTALL_INTERFACE:include/stmm-mynes>  # <prefix>/include/stmm-mynes
)

target_link_libraries(stmm-mynes ${STMMMYNES_EXTRA_LIBRARIES})

set_target_properties(stmm-mynes PROPERTIES ${CMAKE_BUILD_TYPE}_POSTFIX "")
set_target_properties(stmm-mynes PROPERTIES
        VERSION ${STMM_MYNES_VERSION}
        SOVERSION ${STMM_MYNES_MAJOR_VERSION}
#        OUTPUT_NAME "stmm-mynes-${STMM_MYNES_VERSION}"
        )

DefineTargetPublicCompileOptions(stmm-mynes)

include(GNUInstallDirs)
set(STMMI_PKG_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/stmm-games")
# Set version for config.cc.in
set(STMMI_PKG_VERSION "${STMM_MYNES_VERSION}")

# Create config file for library
configure_file("${PROJECT_SOURCE_DIR}/src/stmm-mynes-config.cc.in"
               "${PROJECT_BINARY_DIR}/stmm-mynes-config.cc" @ONLY)

include(FindPkgConfig)
if (PKG_CONFIG_FOUND)
    # Set install dir for pkg-config file
    set(STMMI_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
    # Create pkg-config file for the library
    configure_file("${PROJECT_SOURCE_DIR}/stmm-mynes.pc.in"
                   "${PROJECT_BINARY_DIR}/stmm-mynes.pc" @ONLY)
endif()

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "stmm-mynes was configured with the following options:")
message(STATUS " STMMI_HEADERS:                 ${STMMI_HEADERS}")
message(STATUS " STMMI_SOURCES:                 ${STMMI_SOURCES}")
message(STATUS " STMMMYNES_EXTRA_INCLUDE_DIRS:  ${STMMMYNES_EXTRA_INCLUDE_DIRS}")
message(STATUS " STMMMYNES_EXTRA_INCLUDE_SDIRS: ${STMMMYNES_EXTRA_INCLUDE_SDIRS}")
message(STATUS " STMMMYNES_EXTRA_LIBRARIES:     ${STMMMYNES_EXTRA_LIBRARIES}")
message(STATUS " CMAKE_BUILD_TYPE:              ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:         ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:    ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " CMAKE_CXX_FLAGS:               ${CMAKE_CXX_FLAGS}")
message(STATUS " install prefix:                ${CMAKE_INSTALL_PREFIX}")
message(STATUS " BUILD_DOCS:                    ${BUILD_DOCS}")
message(STATUS " BUILD_TESTING:                 ${BUILD_TESTING}")
endif()

# Documentation
add_subdirectory(doc)
if (BUILD_DOCS)
    # build automatically, otherwise call `make doc`
    add_dependencies(stmm-mynes doc)
endif()

# Testing
enable_testing()
add_subdirectory(test)

install(TARGETS stmm-mynes LIBRARY DESTINATION "lib"  ARCHIVE DESTINATION "lib")

install(FILES ${STMMI_HEADERS} DESTINATION "include/stmm-mynes")

if (PKG_CONFIG_FOUND)
    install(FILES  "${PROJECT_BINARY_DIR}/stmm-mynes.pc"   DESTINATION "lib/pkgconfig")
endif()

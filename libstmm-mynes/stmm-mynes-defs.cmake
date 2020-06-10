# File: libstmm-mynes/stmm-mynes-defs.cmake

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

# Libtool CURRENT/REVISION/AGE: here
#   MAJOR is CURRENT interface
#   MINOR is REVISION (implementation of interface)
#   AGE is always 0
set(STMM_MYNES_MAJOR_VERSION 0)
set(STMM_MYNES_MINOR_VERSION 13) # !-U-!
set(STMM_MYNES_VERSION "${STMM_MYNES_MAJOR_VERSION}.${STMM_MYNES_MINOR_VERSION}.0")

# required stmm-games version
set(STMM_MYNES_REQ_STMM_GAMES_MAJOR_VERSION 0)
set(STMM_MYNES_REQ_STMM_GAMES_MINOR_VERSION 24) # !-U-!
set(STMM_MYNES_REQ_STMM_GAMES_VERSION "${STMM_MYNES_REQ_STMM_GAMES_MAJOR_VERSION}.${STMM_MYNES_REQ_STMM_GAMES_MINOR_VERSION}")

if ("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
    include(FindPkgConfig)
    if (NOT PKG_CONFIG_FOUND)
        message(FATAL_ERROR "Mandatory 'pkg-config' not found!")
    endif()
    # Beware! The prefix passed to pkg_check_modules(PREFIX ...) shouldn't contain underscores!
    pkg_check_modules(STMMGAMES    REQUIRED  stmm-games>=${STMM_MYNES_REQ_STMM_GAMES_VERSION})
endif()

# include dirs
set(        STMMMYNES_EXTRA_INCLUDE_DIRS  "")
set(        STMMMYNES_EXTRA_INCLUDE_SDIRS "")
list(APPEND STMMMYNES_EXTRA_INCLUDE_SDIRS "${STMMGAMES_INCLUDE_DIRS}")

set(STMMI_TEMP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../libstmm-mynes/include")
set(STMMI_TEMP_HEADERS_DIR "${STMMI_TEMP_INCLUDE_DIR}/stmm-mynes")

set(        STMMMYNES_INCLUDE_DIRS  "")
list(APPEND STMMMYNES_INCLUDE_DIRS  "${STMMI_TEMP_INCLUDE_DIR}")
list(APPEND STMMMYNES_INCLUDE_DIRS  "${STMMI_TEMP_HEADERS_DIR}")
list(APPEND STMMMYNES_INCLUDE_DIRS  "${STMMMYNES_EXTRA_INCLUDE_DIRS}")
set(        STMMMYNES_INCLUDE_SDIRS "")
list(APPEND STMMMYNES_INCLUDE_SDIRS "${STMMMYNES_EXTRA_INCLUDE_SDIRS}")

# libs
set(        STMMI_TEMP_EXTERNAL_LIBRARIES       "")
list(APPEND STMMI_TEMP_EXTERNAL_LIBRARIES       "${STMMGAMES_LIBRARIES}")

set(        STMMMYNES_EXTRA_LIBRARIES     "")
list(APPEND STMMMYNES_EXTRA_LIBRARIES     "${STMMI_TEMP_EXTERNAL_LIBRARIES}")

if (BUILD_SHARED_LIBS)
    set(STMMI_LIB_FILE "${PROJECT_SOURCE_DIR}/../libstmm-mynes/build/libstmm-mynes.so")
else()
    set(STMMI_LIB_FILE "${PROJECT_SOURCE_DIR}/../libstmm-mynes/build/libstmm-mynes.a")
endif()

set(        STMMMYNES_LIBRARIES "")
list(APPEND STMMMYNES_LIBRARIES "${STMMI_LIB_FILE}")
list(APPEND STMMMYNES_LIBRARIES "${STMMMYNES_EXTRA_LIBRARIES}")

if ("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
    DefineAsSecondaryTarget(stmm-mynes  ${STMMI_LIB_FILE}  "${STMMMYNES_INCLUDE_DIRS}"  "" "${STMMI_TEMP_EXTERNAL_LIBRARIES}")
endif()

# if (("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "") AND NOT TARGET stmm-mynes)
#     if (BUILD_SHARED_LIBS)
#         add_library(stmm-mynes SHARED IMPORTED)
#     else()
#         add_library(stmm-mynes STATIC IMPORTED)
#     endif()
#     set_target_properties(stmm-mynes PROPERTIES IMPORTED_LOCATION             "${STMMI_LIB_FILE}")
#     set_target_properties(stmm-mynes PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${STMMMYNES_INCLUDE_DIRS}")
#     set_target_properties(stmm-mynes PROPERTIES INTERFACE_LINK_LIBRARIES      "${STMMMYNES_EXTRA_LIBRARIES}")
# endif()

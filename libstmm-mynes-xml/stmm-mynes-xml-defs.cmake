# Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, see <http://www.gnu.org/licenses/>

# File:   stmm-mynes-xml-defs.cmake

# Libtool CURRENT/REVISION/AGE: here
#   MAJOR is CURRENT interface
#   MINOR is REVISION (implementation of interface)
#   AGE is always 0
set(STMM_MYNES_XML_MAJOR_VERSION 0)
set(STMM_MYNES_XML_MINOR_VERSION 18) # !-U-!
set(STMM_MYNES_XML_VERSION "${STMM_MYNES_XML_MAJOR_VERSION}.${STMM_MYNES_XML_MINOR_VERSION}.0")

# required stmm-mynes version
set(STMM_MYNES_XML_REQ_STMM_MYNES_MAJOR_VERSION 0)
set(STMM_MYNES_XML_REQ_STMM_MYNES_MINOR_VERSION 18) # !-U-!
set(STMM_MYNES_XML_REQ_STMM_MYNES_VERSION "${STMM_MYNES_XML_REQ_STMM_MYNES_MAJOR_VERSION}.${STMM_MYNES_XML_REQ_STMM_MYNES_MINOR_VERSION}")

# required stmm-games-xml-game version
set(STMM_MYNES_XML_REQ_STMM_GAMES_XML_GAME_MAJOR_VERSION 0)
set(STMM_MYNES_XML_REQ_STMM_GAMES_XML_GAME_MINOR_VERSION 29) # !-U-!
set(STMM_MYNES_XML_REQ_STMM_GAMES_XML_GAME_VERSION "${STMM_MYNES_XML_REQ_STMM_GAMES_XML_GAME_MAJOR_VERSION}.${STMM_MYNES_XML_REQ_STMM_GAMES_XML_GAME_MINOR_VERSION}")

if ("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
    include(FindPkgConfig)
    if (NOT PKG_CONFIG_FOUND)
        message(FATAL_ERROR "Mandatory 'pkg-config' not found!")
    endif()
    # Beware! The prefix passed to pkg_check_modules(PREFIX ...) shouldn't contain underscores!
    pkg_check_modules(STMMGAMESXMLGAME   REQUIRED  stmm-games-xml-game>=${STMM_MYNES_XML_REQ_STMM_GAMES_XML_GAME_VERSION})
endif()

include("${PROJECT_SOURCE_DIR}/../libstmm-mynes/stmm-mynes-defs.cmake")

# include dirs
set(        STMMMYNESXML_EXTRA_INCLUDE_DIRS  "")
list(APPEND STMMMYNESXML_EXTRA_INCLUDE_DIRS  "${STMMMYNES_INCLUDE_DIRS}")
set(        STMMMYNESXML_EXTRA_INCLUDE_SDIRS "")
list(APPEND STMMMYNESXML_EXTRA_INCLUDE_SDIRS "${STMMMYNES_INCLUDE_SDIRS}")
list(APPEND STMMMYNESXML_EXTRA_INCLUDE_SDIRS "${STMMGAMESXMLGAME_INCLUDE_DIRS}")

set(STMMI_TEMP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../libstmm-mynes-xml/include")
set(STMMI_TEMP_HEADERS_DIR "${STMMI_TEMP_INCLUDE_DIR}/stmm-mynes-xml")

set(        STMMMYNESXML_INCLUDE_DIRS  "")
list(APPEND STMMMYNESXML_INCLUDE_DIRS  "${STMMI_TEMP_INCLUDE_DIR}")
list(APPEND STMMMYNESXML_INCLUDE_DIRS  "${STMMI_TEMP_HEADERS_DIR}")
list(APPEND STMMMYNESXML_INCLUDE_DIRS  "${STMMMYNESXML_EXTRA_INCLUDE_DIRS}")
set(        STMMMYNESXML_INCLUDE_SDIRS "")
list(APPEND STMMMYNESXML_INCLUDE_SDIRS "${STMMMYNESXML_EXTRA_INCLUDE_SDIRS}")

# libs
set(        STMMI_TEMP_EXTERNAL_LIBRARIES       "")
list(APPEND STMMI_TEMP_EXTERNAL_LIBRARIES       "${STMMGAMESXMLGAME_LIBRARIES}")

set(        STMMMYNESXML_EXTRA_LIBRARIES     "")
list(APPEND STMMMYNESXML_EXTRA_LIBRARIES     "${STMMMYNES_LIBRARIES}")
list(APPEND STMMMYNESXML_EXTRA_LIBRARIES     "${STMMI_TEMP_EXTERNAL_LIBRARIES}")

if (BUILD_SHARED_LIBS)
    set(STMMI_LIB_FILE "${PROJECT_SOURCE_DIR}/../libstmm-mynes-xml/build/libstmm-mynes-xml.so")
else()
    set(STMMI_LIB_FILE "${PROJECT_SOURCE_DIR}/../libstmm-mynes-xml/build/libstmm-mynes-xml.a")
endif()

set(        STMMMYNESXML_LIBRARIES "")
list(APPEND STMMMYNESXML_LIBRARIES "${STMMI_LIB_FILE}")
list(APPEND STMMMYNESXML_LIBRARIES "${STMMMYNESXML_EXTRA_LIBRARIES}")

if ("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
    DefineAsSecondaryTarget(stmm-mynes-xml  ${STMMI_LIB_FILE}  "${STMMMYNESXML_INCLUDE_DIRS}"  "stmm-mynes" "${STMMI_TEMP_EXTERNAL_LIBRARIES}")
endif()

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

# File:   mynes-defs.cmake

# Libtool CURRENT/REVISION/AGE: here
#   MAJOR is CURRENT interface
#   MINOR is REVISION (implementation of interface)
#   AGE is always 0
set(MYNES_MAJOR_VERSION 0)
set(MYNES_MINOR_VERSION 20) # !-U-!
set(MYNES_VERSION "${MYNES_MAJOR_VERSION}.${MYNES_MINOR_VERSION}.0")

# required stmm-mynes-xml version
set(MYNES_REQ_STMM_MYNES_XML_MAJOR_VERSION 0)
set(MYNES_REQ_STMM_MYNES_XML_MINOR_VERSION 20) # !-U-!
set(MYNES_REQ_STMM_MYNES_XML_VERSION "${MYNES_REQ_STMM_MYNES_XML_MAJOR_VERSION}.${MYNES_REQ_STMM_MYNES_XML_MINOR_VERSION}")

# required stmm-games-xml-gtk version
set(MYNES_REQ_STMM_GAMES_XML_GTK_MAJOR_VERSION 0)
set(MYNES_REQ_STMM_GAMES_XML_GTK_MINOR_VERSION 31) # !-U-!
set(MYNES_REQ_STMM_GAMES_XML_GTK_VERSION "${MYNES_REQ_STMM_GAMES_XML_GTK_MAJOR_VERSION}.${MYNES_REQ_STMM_GAMES_XML_GTK_MINOR_VERSION}")

# required stmm-input-gtk-dm version
set(MYNES_REQ_STMM_INPUT_GTK_DM_MAJOR_VERSION 0)
set(MYNES_REQ_STMM_INPUT_GTK_DM_MINOR_VERSION 16) # !-U-!
set(MYNES_REQ_STMM_INPUT_GTK_DM_VERSION "${MYNES_REQ_STMM_INPUT_GTK_DM_MAJOR_VERSION}.${MYNES_REQ_STMM_INPUT_GTK_DM_MINOR_VERSION}")

if ("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
    include(FindPkgConfig)
    if (NOT PKG_CONFIG_FOUND)
        message(FATAL_ERROR "Mandatory 'pkg-config' not found!")
    endif()
    # Beware! The prefix passed to pkg_check_modules(PREFIX ...) shouldn't contain underscores!
    pkg_check_modules(STMMGAMESXMLGTK  REQUIRED  stmm-games-xml-gtk>=${MYNES_REQ_STMM_GAMES_XML_GTK_VERSION})
    pkg_check_modules(STMMINPUTGTKDM   REQUIRED  stmm-input-gtk-dm>=${MYNES_REQ_STMM_INPUT_GTK_DM_VERSION})
endif()

include("${PROJECT_SOURCE_DIR}/../libstmm-mynes-xml/stmm-mynes-xml-defs.cmake")

# include dirs
set(        MYNES_EXTRA_INCLUDE_DIRS  "")
list(APPEND MYNES_EXTRA_INCLUDE_DIRS  "${STMMMYNESXML_INCLUDE_DIRS}")
set(        MYNES_EXTRA_INCLUDE_SDIRS "")
list(APPEND MYNES_EXTRA_INCLUDE_SDIRS "${STMMMYNESXML_INCLUDE_SDIRS}")
list(APPEND MYNES_EXTRA_INCLUDE_SDIRS "${STMMGAMESXMLGTK_INCLUDE_DIRS}")
list(APPEND MYNES_EXTRA_INCLUDE_SDIRS "${STMMINPUTGTKDM_INCLUDE_DIRS}")

# libs
set(        MYNES_EXTRA_LIBRARIES     "")
list(APPEND MYNES_EXTRA_LIBRARIES     "${STMMMYNESXML_LIBRARIES}")
list(APPEND MYNES_EXTRA_LIBRARIES     "${STMMGAMESXMLGTK_LIBRARIES}")
list(APPEND MYNES_EXTRA_LIBRARIES     "${STMMINPUTGTKDM_LIBRARIES}")

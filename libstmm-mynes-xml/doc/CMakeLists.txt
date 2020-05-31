# stmm-mynes-xml: doc/CMakeLists.txt

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

include(FindDoxygen)

if (DOXYGEN_FOUND)
    if (NOT DOXYGEN_DOT_FOUND) 
        message(WARNING "Graphviz doesn't seem to be installed. Doxygen will not be able to generate graphs. Consider installing this package.")
    endif(NOT DOXYGEN_DOT_FOUND) 

    include(CommonDoxy)
    set(STMMI_TEMP_DOC_LIB_LIST "stmm-mynes")
    list(APPEND STMMI_TEMP_DOC_LIB_LIST "stmm-games;stmm-games-file;stmm-games-xml-base;stmm-games-xml-game")
    list(APPEND STMMI_TEMP_DOC_LIB_LIST "stmm-input-ev;stmm-input-base;stmm-input")
    CreateLibDoxy(stmm-mynes-xml  ${STMM_MYNES_XML_VERSION} "${STMMI_TEMP_DOC_LIB_LIST}")
else(DOXYGEN_FOUND)
    add_custom_target(doc COMMAND echo "Doxygen not found")
endif(DOXYGEN_FOUND)

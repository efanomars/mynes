# mynes: test/CMakeLists.txt

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

include(CommonTesting)

option(BUILD_TESTING "Build tests" OFF)

if (BUILD_TESTING)
    include(GNUInstallDirs)
    set(STMMI_TESTING_PKG_DATA_DIR      "${CMAKE_INSTALL_FULL_DATADIR}/stmm-games")

    set(STMMI_TESTING_RO_DATA_DIR_1     "/usr/local/${CMAKE_INSTALL_DATADIR}/stmm-games")
    set(STMMI_TESTING_RO_DATA_DIR_2     "/usr/${CMAKE_INSTALL_DATADIR}/stmm-games")
    set(STMMI_TESTING_RO_DATA_DIR_3     "${STMMI_TESTING_PKG_DATA_DIR}")

    # Test dirs and paths, used by testconfig.cc.in configured in TestFiles function
    set(STMMI_TESTING_BINARY_DIR  "${PROJECT_BINARY_DIR}/test")
    set(STMMI_TESTING_GAME_FILES_PATHS  "${STMMI_MYN_DATA_GAMES_FILES}")
    string(REPLACE ";" "\\n\"\\\n\"" STMMI_TESTING_GAME_FILES_PATHS "${STMMI_TESTING_GAME_FILES_PATHS}")
    set(STMMI_TESTING_THEME_FILES_PATHS
            "${PROJECT_SOURCE_DIR}/../libstmm-mynes-xml/data/themes/mynes/simple_m.thm/theme.xml"
            "${STMMI_TESTING_RO_DATA_DIR_1}/themes/simple.thm/theme.xml"
            "${STMMI_TESTING_RO_DATA_DIR_2}/themes/simple.thm/theme.xml"
            "${STMMI_TESTING_RO_DATA_DIR_3}/themes/simple.thm/theme.xml"
           )
    string(REPLACE ";" "\\n\"\\\n\"" STMMI_TESTING_THEME_FILES_PATHS "${STMMI_TESTING_THEME_FILES_PATHS}")
    # Test dirs
    set(STMMI_TEST_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/test")
    # Test sources should end with .cxx
    set(STMMI_TEST_SOURCES
             "${STMMI_TEST_SOURCES_DIR}/testLoadGames.cxx"
           )

    set(TEST_MYNES_REQ_STMM_GAMES_FAKE_VERSION "0.25") # !-U-!
    set(TEST_MYNES_REQ_STMM_INPUT_FAKE_VERSION "0.14") # !-U-!

    include(FindPkgConfig)
    if (NOT PKG_CONFIG_FOUND)
        message(FATAL_ERROR "Mandatory 'pkg-config' not found!")
    endif()
    # Beware! The prefix passed to pkg_check_modules(PREFIX ...) shouldn't contain underscores!
    pkg_check_modules(STMMGAMESFAKE  REQUIRED  stmm-games-fake>=${TEST_MYNES_REQ_STMM_GAMES_FAKE_VERSION})
    pkg_check_modules(STMMINPUTFAKE  REQUIRED  stmm-input-fake>=${TEST_MYNES_REQ_STMM_INPUT_FAKE_VERSION})

    set(        STMMI_TEST_EXTRA_INCLUDES "")
    list(APPEND STMMI_TEST_EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/src")
    list(APPEND STMMI_TEST_EXTRA_INCLUDES "${MYNES_EXTRA_INCLUDE_DIRS}")
    list(APPEND STMMI_TEST_EXTRA_INCLUDES "${MYNES_EXTRA_INCLUDE_SDIRS}")
    list(APPEND STMMI_TEST_EXTRA_INCLUDES "${STMMINPUTFAKE_INCLUDE_DIRS}")
    list(APPEND STMMI_TEST_EXTRA_INCLUDES "${STMMGAMESFAKE_INCLUDE_DIRS}")
    set(        STMMI_TEST_EXTRA_LIBRARIES "")
    list(APPEND STMMI_TEST_EXTRA_LIBRARIES "${STMMINPUTFAKE_LIBRARIES}")
    list(APPEND STMMI_TEST_EXTRA_LIBRARIES "${STMMGAMESFAKE_LIBRARIES}")
    list(APPEND STMMI_TEST_EXTRA_LIBRARIES "${MYNES_EXTRA_LIBRARIES}")
    set(        STMMI_TEST_EXTRA_SOURCES "")
    list(APPEND STMMI_TEST_EXTRA_SOURCES "${PROJECT_SOURCE_DIR}/src/setupstdconfig.h")
    list(APPEND STMMI_TEST_EXTRA_SOURCES "${PROJECT_SOURCE_DIR}/src/setupstdconfig.cc")
    list(APPEND STMMI_TEST_EXTRA_SOURCES "${PROJECT_SOURCE_DIR}/src/setupxmlgameloader.h")
    list(APPEND STMMI_TEST_EXTRA_SOURCES "${PROJECT_SOURCE_DIR}/src/setupxmlgameloader.cc")
    list(APPEND STMMI_TEST_EXTRA_SOURCES "${PROJECT_SOURCE_DIR}/src/setupxmlthemeloader.h")
    list(APPEND STMMI_TEST_EXTRA_SOURCES "${PROJECT_SOURCE_DIR}/src/setupxmlthemeloader.cc")

    TestFiles("${STMMI_TEST_SOURCES}" "${STMMI_TEST_EXTRA_SOURCES}" "${STMMI_TEST_EXTRA_INCLUDES}" "${STMMI_TEST_EXTRA_LIBRARIES}")

    include(CTest)
endif()

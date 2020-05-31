# File: scripts/priv/debian_src_versions.cmake

# This has to be run with cmake -DSTMMI_PROJECT_SOURCE_DIR="/home/.../libstmm-mynes" ... -P debian_src_versions.cmake

# Parameters:
#   STMMI_DEBIAN_SRC_FILENAME     The template file name in ${STMMI_DEBIAN_SRC_FROM_DIR}
#                                 without the ".in" suffix.
#                                 (ex. if value is "control" the file name is "control.in")
#                                 (ex. "libstmm-mynes@STMM_MYNES_MAJOR_VERSION@.install"
#   STMMI_DEBIAN_SRC_FROM_DIR     The absolute path to the directory containing file
#                                 "${STMMI_DEBIAN_SRC_FILENAME}.in"
#                                 (ex. .../mynes/scripts/priv/debian.orig)
#   STMMI_DEBIAN_SRC_TO_DIR       The absolute path to the target directory
#                                 (ex. /tmp/mynesDeb/mynes-0.1/debian)
#   STMMI_DEBIAN_SRC_TMP_1        The absolute path to a unique temporary file used by this function.
#                                 If empty don't configure file name itself.
#   STMMI_DEBIAN_SRC_TMP_2        The absolute path to a another unique temporary file used by this function.
#   STMMI_PROJECT_SOURCE_DIR      The absolute path to one of the subproject directories

set(PROJECT_SOURCE_DIR "${STMMI_PROJECT_SOURCE_DIR}")

include("${PROJECT_SOURCE_DIR}/../mynes/mynes-defs.cmake")

if ("${STMMI_DEBIAN_SRC_TMP_1}" STREQUAL "")
    set(STMMI_DEBIAN_SRC_RESNAME "${STMMI_DEBIAN_SRC_FILENAME}")
else()
    # write the template file name to the temporary
    file(WRITE "${STMMI_DEBIAN_SRC_TMP_1}" "${STMMI_DEBIAN_SRC_FILENAME}")
    # sobstitute version numbers if file name contains any
    configure_file("${STMMI_DEBIAN_SRC_TMP_1}" "${STMMI_DEBIAN_SRC_TMP_2}" @ONLY)
    # read the destination file name from new temporary
    file(STRINGS "${STMMI_DEBIAN_SRC_TMP_2}" STMMI_DEBIAN_SRC_RESNAME)
endif()

configure_file("${STMMI_DEBIAN_SRC_FROM_DIR}/${STMMI_DEBIAN_SRC_FILENAME}.in"
               "${STMMI_DEBIAN_SRC_TO_DIR}/${STMMI_DEBIAN_SRC_RESNAME}" @ONLY)

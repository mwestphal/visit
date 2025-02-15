# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#-----------------------------------------------------------------------------
# Runtime loading of Mesa relies on having OSMesa in a known path.
#
# For development builds we create symlink to libOSMesa along side
# VisIt's libraries.
#
# I (Cyrus) am not sure how this will work on windows - there may be a
# more general solution.
#
# For a 'make install'ed version libOSMesa should be properly installed
# by our install targets.
#
# Modifications:
#    Jeremy Meredith, Wed Jan 20 11:27:30 EST 2010
#    I added a command to put a symlink to python in the lib directory.
#    We need this to set python-related environment variables in
#    internallauncher.  I don't know if this is a good solution, as I
#    don't know the implication for an installed setup -- this is only
#    intended to fix running inside a development build.
#
#    Cyrus Harrison, Wed Feb  3 10:10:22 PST 2010
#    Add symlinks to allow us to locate the vtk python wrapper module.
#
#    Kathleen Bonnell, Wed Mar 17 10:08:17 MST 2010 
#    Use FILE(COPY ...) on windows, as windows doesn't support symlinks.
#    Handled python filters on windows differently, as the VTK directory 
#    structure is different.
#
#    Cyrus Harrison, Wed Jul  7 10:09:07 PDT 2010
#    Remove any existing symlinks before attempting to create new ones.
#
#    Kathleen Biagas, Thu May 10 10:58:39 MST 2012 
#    Change how python is handled on Windows to reflect recent changes.
#
#    Kathleen Biagas, Thu June 14 15:53:12 MST 2012 
#    Add target for pyside on windows.
#
#    Gunther H. Weber, Thu Nov 21 18:26:51 PST 2013
#    Use ${PYSIDE_PYTHONPATH} (set by findpackage PySide) instead of 
#    ${VISIT_PYSIDE_DIR}/lib/site-packages to support system PySide installs
#
#    Kathleen Biagas, Thu Jun 25 14:13:46 PDT 2020
#    Check if Pyside installed in python before adding to lib/site-packages.
#
#    Kathleen Biagas, Tue Jan 31, 2023
#    visit_special_builds renamed to visit_util_builds.
#
#    Kathleen Biagas, Wed Feb 28, 2024
#    Add direct install of build/lib/site-packages dir.
#    Change conduit-python-libs from using symlink to direct copy, to play
#    well with new direct install of build/lib/site-packages dir.
#
#----------------------------------------------------------------------------

IF(VISIT_PYTHON_DIR)
  IF (NOT WIN32)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove -f
                            ${CMAKE_CURRENT_BINARY_DIR}/python)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink
                            ${VISIT_PYTHON_DIR}
                            ${CMAKE_CURRENT_BINARY_DIR}/python)
  ELSE (NOT WIN32)

    # because you don't need to choose your configuration up-front
    # on windows, make this a custom target so they are copied to the
    # correct lib/${CFG} directory
    ADD_CUSTOM_TARGET(python_lib ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${VISIT_PYTHON_DIR}/Lib
            ${VISIT_LIBRARY_DIR}/python/Lib
        COMMENT "Copying ${VISIT_PYTHON_DIR}/Lib/ ${VISIT_LIBRARY_DIR}/python/Lib")
    visit_add_to_util_builds(python_lib)
  ENDIF (NOT WIN32)
ENDIF()

# prepare site-packages

# is this the correct if-test ?
if(VISIT_PYTHON_FILTERS OR VISIT_PYTHON_SCRIPTING)

    # there were unresolvable issues with the INSTALL(CODE ...
    # in FindVisItPython for pip-installed modules
    # this direct copy of the lib/site-packages dir from the
    # build works.

    install(DIRECTORY ${VISIT_LIBRARY_DIR}/site-packages
            DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/
            FILE_PERMISSIONS OWNER_WRITE OWNER_READ
                             GROUP_WRITE GROUP_READ
                                         WORLD_READ
            DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                                  GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                              WORLD_READ WORLD_EXECUTE)

endif()


IF(VISIT_PYTHON_FILTERS OR (VISIT_PYTHON_SCRIPTING AND HAVE_LIBPYSIDE))
    IF(NOT WIN32)
        EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory
                        ${CMAKE_CURRENT_BINARY_DIR}/site-packages
                        RESULT_VARIABLE LIB_SITE_PKGS_MKDIR)
        #
        # make sure lib/site-packages exists before we try to
        # symlink or copy the vtk python wrappers
        #
        IF(NOT "${LIB_SITE_PKGS_MKDIR}" STREQUAL "0")
            MESSAGE(WARNING "Failed to create lib/site-packages/")
        ENDIF(NOT "${LIB_SITE_PKGS_MKDIR}" STREQUAL "0")
    ENDIF(NOT WIN32)
ENDIF(VISIT_PYTHON_FILTERS OR (VISIT_PYTHON_SCRIPTING AND HAVE_LIBPYSIDE))

IF(VISIT_PYTHON_FILTERS)
# We need to be able to locate the vtk python wrapper lib at run time.
# Place symlinks to its components in lib/site-packages/
    MESSAGE(STATUS "Adding VTK python wrappers to lib/site-packages/")
    add_custom_target(vtk_python_modules ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${VTK_PY_WRAPPERS_DIR}
                ${VISIT_LIBRARY_DIR}/site-packages
        COMMENT "Copying ${VTK_PY_WRAPPERS_DIR}/vtkmodules to ${VISIT_LIBRARY_DIR}/site-packages/vtkmodules")
    if(WIN32)
        visit_add_to_util_builds(vtk_python_modules)
    endif()
ENDIF(VISIT_PYTHON_FILTERS)

if(VISIT_PYTHON_SCRIPTING AND HAVE_LIBPYSIDE AND NOT PYSIDE_IN_PYTHON)
    function(SYMLINK_PYSIDE pkg)
        get_filename_component(ITM_BASE ${${pkg}} NAME)
        set(sym_cmd "create_symlink")
        if(WIN32)
            set(sym_cmd "copy_directory")
        endif()
        # KSB to do, check if this works on Windows
        message(STATUS "${sym_cmd} ${${pkg}} to ${CMAKE_CURRENT_BINARY_DIR}/site-packages/${ITM_BASE}")
        execute_process(COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/site-packages/${ITM_BASE}
            RESULT_VARIABLE PYSIDE_MODULE_SYM_RM)
        execute_process(COMMAND ${CMAKE_COMMAND} -E ${sym_cmd}
            ${${pkg}} ${CMAKE_CURRENT_BINARY_DIR}/site-packages/${ITM_BASE}
            RESULT_VARIABLE PYSIDE_MODULE_SYM_CREATE)
        if(NOT "${PYSIDE_MODULE_SYM_CREATE}" STREQUAL "0")
            message(WARNING "Failed to create PySide module in lib/site-packages/ for ${${pkg}}")
        endif()
        unset(sym_cmd)
    endfunction()

    message(STATUS "Adding PySide to lib/site-packages/")
    SYMLINK_PYSIDE(PYSIDE_PYTHONPATH)
    SYMLINK_PYSIDE(SHIBOKEN_PYTHON_MODULE_DIR)
endif()

if(VISIT_PYTHON_SCRIPTING AND CONDUIT_DIR)
    # symlink conduit python module so it works during dev builds
    message(STATUS "Adding Conduit python module to lib/site-packages/")

    
    set(CONDUIT_PY_MODULE ${CONDUIT_DIR}/python-modules/conduit)
    if(NOT EXISTS ${CONDUIT_PY_MODULE})
        message(WARNING "Failed to find Conduit Python Module at ${CONDUIT_PY_MODULE}")
    else()
        add_custom_target(conduit_python_module ALL
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CONDUIT_PY_MODULE}
                    ${VISIT_LIBRARY_DIR}/site-packages/conduit
            COMMENT "Copying ${CONDUIT_PY_MODULE} to ${VISIT_LIBRARY_DIR}/site-packages/conduit"
        )
        if(WIN32)
            visit_add_to_util_builds(conduit_python_module)
        endif()
    endif()
endif()


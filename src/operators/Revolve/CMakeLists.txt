# DO NOT EDIT THIS FILE! THIS FILE IS AUTOMATICALLY GENERATED BY xml2cmake
PROJECT(Revolve_operator)

ADD_OPERATOR_CODE_GEN_TARGETS(Revolve)

SET(COMMON_SOURCES
RevolvePluginInfo.C
RevolveCommonPluginInfo.C
RevolveAttributes.C
)

SET(LIBI_SOURCES
RevolvePluginInfo.C
)

SET(LIBG_SOURCES
RevolveGUIPluginInfo.C
QvisRevolveWindow.C
${COMMON_SOURCES}
)

SET(LIBV_SOURCES
RevolveViewerEnginePluginInfo.C
RevolveViewerPluginInfo.C
${COMMON_SOURCES}
)

SET(LIBE_SOURCES
RevolveViewerEnginePluginInfo.C
RevolveEnginePluginInfo.C
avtRevolveFilter.C
${COMMON_SOURCES}
)


INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_OPERATOR_INCLUDES}
)

LINK_DIRECTORIES(${VISIT_LIBRARY_DIR} )

ADD_LIBRARY(IRevolveOperator ${LIBI_SOURCES})
TARGET_LINK_LIBRARIES(IRevolveOperator visitcommon)
SET(INSTALLTARGETS IRevolveOperator)

IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    ADD_LIBRARY(GRevolveOperator ${LIBG_SOURCES})
    set_target_properties(GRevolveOperator PROPERTIES AUTOMOC ON)
    TARGET_LINK_LIBRARIES(GRevolveOperator visitcommon gui )

    ADD_LIBRARY(VRevolveOperator ${LIBV_SOURCES})
    ADD_TARGET_DEFINITIONS(VRevolveOperator VIEWER)
    TARGET_LINK_LIBRARIES(VRevolveOperator visitcommon viewer )

    SET(INSTALLTARGETS ${INSTALLTARGETS} GRevolveOperator VRevolveOperator)

    IF(VISIT_PYTHON_SCRIPTING)
        SET(LIBS_SOURCES
            RevolveScriptingPluginInfo.C
            PyRevolveAttributes.C
            ${COMMON_SOURCES}
        )
        ADD_LIBRARY(SRevolveOperator ${LIBS_SOURCES})
        TARGET_LINK_LIBRARIES(SRevolveOperator visitcommon visitpy ${PYTHON_LIBRARY})
        SET(INSTALLTARGETS ${INSTALLTARGETS} SRevolveOperator)
    ENDIF(VISIT_PYTHON_SCRIPTING)

    IF(VISIT_JAVA)
        FILE(COPY RevolveAttributes.java DESTINATION ${JavaClient_BINARY_DIR}/src/operators)
        ADD_CUSTOM_TARGET(JavaRevolve ALL ${Java_JAVAC_EXECUTABLE} ${VISIT_Java_FLAGS} -d ${JavaClient_BINARY_DIR} -classpath ${JavaClient_BINARY_DIR} -sourcepath ${JavaClient_BINARY_DIR} RevolveAttributes.java
            DEPENDS JavaClient
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    ENDIF(VISIT_JAVA)
ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)

ADD_LIBRARY(ERevolveOperator_ser ${LIBE_SOURCES})
TARGET_LINK_LIBRARIES(ERevolveOperator_ser visitcommon avtpipeline_ser avtexpressions_ser avtfilters_ser )
SET(INSTALLTARGETS ${INSTALLTARGETS} ERevolveOperator_ser)

IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(ERevolveOperator_par ${LIBE_SOURCES})
    TARGET_LINK_LIBRARIES(ERevolveOperator_par visitcommon avtpipeline_par avtexpressions_par avtfilters_par )
    SET(INSTALLTARGETS ${INSTALLTARGETS} ERevolveOperator_par)
ENDIF(VISIT_PARALLEL)

VISIT_INSTALL_OPERATOR_PLUGINS(${INSTALLTARGETS})
VISIT_PLUGIN_TARGET_OUTPUT_DIR(operators ${INSTALLTARGETS})
VISIT_PLUGIN_TARGET_FOLDER(operators Revolve ${INSTALLTARGETS})


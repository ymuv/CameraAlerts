project(${HELLO_EXAMPLE_MAIN})

find_package(Qt5Widgets)

add_definitions(-D__INSTALL_BIN_DIR__="${CMAKE_INSTALL_PREFIX}/${MAIN_BIN_DIR}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set (MyProjectLib_ui  mainwindow.ui)

qt5_wrap_ui (MyProjectLib_ui_moc  ${MyProjectLib_ui})

include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5TextToSpeech_INCLUDE_DIRS})

add_executable( ${HELLO_EXAMPLE_MAIN} main.cpp ${MyProjectLib_ui_moc} mainwindow.cpp)

target_link_libraries (${HELLO_EXAMPLE_MAIN} ${Qt5Widgets_LIBRARIES} ${Qt5TextToSpeech_LIBRARIES})
TARGET_LINK_LIBRARIES(${HELLO_EXAMPLE_MAIN}  ${TEXT_TO_SPEECH_LIB_NAME} )

INSTALL (
    TARGETS ${HELLO_EXAMPLE_MAIN}
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    )

INSTALL (
    FILES  ${CONFIG_FILE}
    DESTINATION ${INSTALL_BIN_DIR}
    )




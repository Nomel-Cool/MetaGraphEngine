cmake_minimum_required(VERSION 3.21)

project(QtGraphEngineKit LANGUAGES C CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

# Qt location. Prefer passing -DCMAKE_PREFIX_PATH=.../msvc2022_64.
find_package(Qt6 6.2 COMPONENTS Core Gui Qml Quick QuickControls2 Network REQUIRED)

file(GLOB SOURCES "cpps/*.c" "cpps/*.cpp")
file(GLOB HEADERS "headers/*.h" "headers/*.hpp")
file(GLOB LIB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/libs/*.lib")
file(GLOB DLL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dlls/*.dll")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/headers)

qt6_add_resources(RESOURCES resources/qml.qrc)
qt6_add_resources(RESOURCES resources/img.qrc)
qt6_add_resources(RESOURCES resources/xml.qrc)

qt_add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC
    ${LIB_FILES}
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    QT_QML_IMPORT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/qmls"
)

qt6_add_qml_module(${PROJECT_NAME} URI "CustomModules" VERSION 1.0 SOURCES ${HEADERS} ${SOURCES})

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install")
set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}/bin")

install(
    TARGETS ${PROJECT_NAME}
    CONFIGURATIONS Debug
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/ DESTINATION ${CMAKE_INSTALL_BINDIR}/resources)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shaders_codes/ DESTINATION ${CMAKE_INSTALL_BINDIR}/shaders_codes)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders_codes ${CMAKE_BINARY_DIR}/shaders_codes
)

foreach(DLL ${DLL_FILES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL} "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )
    install(FILES ${DLL} DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()

# Deploy Qt runtime, platform plugins and QML imports into install/bin.
if (COMMAND qt_generate_deploy_qml_app_script)
    qt_generate_deploy_qml_app_script(
        TARGET ${PROJECT_NAME}
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
        DEPLOY_TOOL_OPTIONS --no-translations
    )
    install(SCRIPT ${deploy_script})
endif()
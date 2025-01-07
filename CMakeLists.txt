cmake_minimum_required(VERSION 3.14)

project(QtGraphEngineKit)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 设置 C++ 标准为 C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# RuntimeLibrary设置，应当所有库保持一致
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

# 设置Qt路径，如果你使用的是系统安装的Qt，你可以省略这一行
#set(CMAKE_PREFIX_PATH "path/to/your/qt")

find_package(Qt6 COMPONENTS Core Gui Qml Quick REQUIRED)

# 使用 file(GLOB ...) 捕获所有源文件和头文件 以及 链接dlls和libs目录中的库文件
file(GLOB SOURCES "cpps/*.c" "cpps/*.cpp")
file(GLOB HEADERS "headers/*.h" "headers/*.hpp")
# 值得注意的是这里千万不要去链接把dll当lib链接了（也就是不用去找dll的位置，只需要加载lib，对应就会找到dll了，亲测即使不同名也可以）
file(GLOB LIB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/libs/*.lib") 
file(GLOB DLL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dlls/*.dll")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/headers)

# 如果你有资源文件(.qrc)
qt6_add_resources(RESOURCES resources/qml.qrc)
qt6_add_resources(RESOURCES resources/img.qrc)
qt6_add_resources(RESOURCES resources/xml.qrc)

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})


# 链接Qt库，使用关键字签名
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIB_FILES} Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick)


# 添加QML导入路径
set_target_properties(${PROJECT_NAME} PROPERTIES
    QT_QML_IMPORT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/qmls"
)

# 添加自定义模块，即QML调用C++类成员函数情形下，注册的C++类所自定义的模块名同步
qt6_add_qml_module(${PROJECT_NAME} URI "CustomModules" VERSION 1.0 SOURCES ${HEADERS} ${SOURCES})

# 设置安装目录，是最终打包的资源可运行程序以及它调用的资源文件，一般打包给客户
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install")
set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}/bin")

# 安装目标（Debug）
install(
    TARGETS ${PROJECT_NAME}
    CONFIGURATIONS Debug
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# 复制资源文件到安装目录
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/ DESTINATION ${CMAKE_INSTALL_BINDIR}/resources)

# 复制着色器代码文件到安装目录
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shaders_codes/ DESTINATION ${CMAKE_INSTALL_BINDIR}/shaders_codes)

# 将资源文件复制到 build 目录，供在build下使用VS打开.sln调试时使用 
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources
)

# 复制着色器代码到 build 输出目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders_codes ${CMAKE_BINARY_DIR}/shaders_codes
)

# 复制每个 .dll 文件到 build 输出目录
foreach(DLL ${DLL_FILES}) 
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL} "$<TARGET_FILE_DIR:${PROJECT_NAME}>" 
) 
endforeach()

# 安装每个 .dll 文件到安装目录 
foreach(DLL ${DLL_FILES}) 
    install(FILES ${DLL} DESTINATION ${CMAKE_INSTALL_BINDIR}
)
endforeach()
add_executable(launcher src/LauncherApp.cpp src/LauncherLayer.cpp)

# 指定输出目录
set_target_properties(launcher PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

target_link_libraries(launcher PRIVATE engine)

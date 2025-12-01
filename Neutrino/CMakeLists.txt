add_executable(runtime src/NeutrinoApp.cpp src/NeutrinoLayer.cpp)

# 指定输出目录
set_target_properties(runtime PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

target_link_libraries(runtime PRIVATE engine)

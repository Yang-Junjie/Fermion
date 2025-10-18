add_executable(game GameApp.cpp)

# 指定输出目录
set_target_properties(game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

target_link_libraries(game PRIVATE engine)

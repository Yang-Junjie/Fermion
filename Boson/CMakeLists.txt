add_executable(BosonEditor BosonEditor.cpp BosonLayer.cpp)

# 指定输出目录
set_target_properties(BosonEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

target_link_libraries(BosonEditor PRIVATE engine)

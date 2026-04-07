add_executable(VulkanFrameLoop
    src/VulkanFrameLoopApp.cpp
)

set_target_properties(VulkanFrameLoop PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

target_link_libraries(VulkanFrameLoop PRIVATE engine)

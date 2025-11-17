

set(BOSON_SOUCES
    ${CMAKE_CURRENT_LIST_DIR}/src/BosonEditor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/BosonLayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Panels/SceneHierarchyPanel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Panels/ContentBrowserPanel.cpp
)

add_executable(BosonEditor ${BOSON_SOUCES})

set_target_properties(BosonEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}

)

target_include_directories(BosonEditor PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/external/ImGuizmo)

target_link_libraries(BosonEditor PRIVATE engine)


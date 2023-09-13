panda_add_executable(ets_docgen main.cpp)

panda_target_link_libraries(ets_docgen es2panda-lib)

panda_target_include_directories(ets_docgen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
panda_target_include_directories(ets_docgen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

panda_target_compile_definitions(ets_docgen
  PRIVATE "DEFAULT_ARKTSCONFIG=\"${GENERATED_DIR}/arktsconfig.json\""
)

panda_add_sanitizers(TARGET ets_docgen SANITIZERS ${PANDA_SANITIZERS_LIST})

add_custom_command(
  COMMENT "Copying ets_doc2rst to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ets_doc2rst.rb"
  OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ets_doc2rst.rb"
  COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/../docgen/render.rb" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ets_doc2rst.rb"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../docgen/render.rb"
)
add_custom_target(es2panda_copy_ets_doc2rst
  DEPENDS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ets_doc2rst.rb"
)
add_dependencies(ets_docgen es2panda_copy_ets_doc2rst)

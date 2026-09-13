add_executable(test_generated_code
test_generated_code.cpp
)

add_executable(${PROJECT_NAME}
ecs.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
ecsCompilerLib
)

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "The ecs compiler path: $<TARGET_FILE:${PROJECT_NAME}>"
)

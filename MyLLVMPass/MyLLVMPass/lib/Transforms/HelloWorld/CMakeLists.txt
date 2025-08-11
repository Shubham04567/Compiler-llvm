include(AddLLVM)

add_llvm_pass_plugin(HelloWorld
  ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld.cpp
)

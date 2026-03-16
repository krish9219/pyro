cmake_minimum_required(VERSION 3.14)
project(pyro VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pyro compiler executable
add_executable(pyro
    src/main.cpp
    src/lexer/lexer.cpp
    src/lexer/token.cpp
    src/parser/parser.cpp
    src/parser/ast.cpp
    src/codegen/codegen.cpp
    src/codegen/cpp_emitter.cpp
    src/runtime/runtime.cpp
)

target_include_directories(pyro PRIVATE src)

# Install
install(TARGETS pyro DESTINATION bin)
install(DIRECTORY stdlib/ DESTINATION lib/pyro/stdlib)

# LSP server
add_executable(pyro-lsp
    src/lsp/lsp_server.cpp
    src/lexer/lexer.cpp
    src/lexer/token.cpp
    src/parser/parser.cpp
    src/parser/ast.cpp
)
target_include_directories(pyro-lsp PRIVATE src)
install(TARGETS pyro-lsp DESTINATION bin)

# Tests
enable_testing()
add_executable(pyro_tests
    tests/test_main.cpp
    src/lexer/lexer.cpp
    src/lexer/token.cpp
    src/parser/parser.cpp
    src/parser/ast.cpp
    src/codegen/codegen.cpp
    src/codegen/cpp_emitter.cpp
    src/runtime/runtime.cpp
)
target_include_directories(pyro_tests PRIVATE src)
add_test(NAME PyroTests COMMAND pyro_tests)

cmake_minimum_required(3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(tklc
    VERSION 1.0
    LANGUAGES CXX
)

add_executable(tklc
    
    lexer/lexer.cxx
    semantic/semantic.cxx
    file/file.cxx
    parser/parser.cxx
    semantic/analyzer/analyzer.cxx
    semantic/symboltable/smb.cxx
    semantic/symbol/symbol.cxx
    semantic/Type/type.cxx
    semantic/analyzer/analyzer.cxx
)

target_include_directories(tklc PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/semantic
    ${CMAKE_CURRENT_SOURCE_DIR}/file
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
)



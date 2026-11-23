cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(tklc
    VERSION 1.0
    LANGUAGES CXX
)

add_executable(tklc

    ir/ir.cxx
    lexer/lexer.cxx
    parser/memory.cxx
    
    file/file.cxx
    parser/parser.cxx
    semantic/analyzer/analyzer.cxx
    semantic/symboltable/smb.cxx
    semantic/symbol/symbol.cxx
    semantic/Type/type.cxx
    
)

target_include_directories(tklc PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/semantic
    ${CMAKE_CURRENT_SOURCE_DIR}/file
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    ${CMAKE_CURRENT_SOURCE_DIR}/ir
    ${CMAKE_CURRENT_SOURCE_DIR}
)



include(CTest)

if(BUILD_TESTING)

    add_executable(lexer_test
        tests/test_lexer.cxx
        lexer/lexer.cxx
        file/file.cxx
    )
    
    add_test(
        NAME lexer_test
        COMMAND lexer_test
    )


    target_include_directories(lexer_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lexer
        ${CMAKE_CURRENT_SOURCE_DIR}/file
    )

    
endif()
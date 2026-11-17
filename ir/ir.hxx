#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <optional>



enum class IRcode {
    ADD,
    SUB,
    MUL,
    DIV,

    LOAD,
    STORE,

    CALL,
    RET,

    JMP,
    BR,

    CONST
};

enum class IRType {
    VOID,
    I8,
    I16,
    I32,
    I64,
    F32,
    F64,
    PTR
};

struct IRInstruction {
    IRcode opcode;

    std::optional<IRValue> result;
    std::vector<IRValue> operands;
};

struct IRValue {
    uint32_t id;
    IRType type;
};

struct BasicBlock {
    std::vector<IRInstruction> instructions;
};

struct IRFunction {
    std::string name;
    std::vector<BasicBlock> blocks;
};

struct IRModule {
    std::vector<IRFunction> functions;
};

struct IRParameter {
    std::string name;
    IRType type;
    IRValue value;
};
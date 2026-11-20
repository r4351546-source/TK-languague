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

struct IRValue {
    uint32_t id;
    IRType type;
};


struct IRParameter {
    std::string name;
    IRType type;
    IRValue value;
};

struct IRInstruction {
    IRcode opcode;

    std::optional<IRValue> result;
    std::vector<IRValue> operands;

    std::vector<uint32_t> targets;
};


struct BasicBlock {
    std::vector<IRInstruction> instructions;
};

struct IRFunction {
    std::string name;

    IRType returnType = IRType::VOID;

    std::vector<IRParameter> parameters;
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
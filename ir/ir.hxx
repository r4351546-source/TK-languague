#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include <variant>

#include <variant>

using IRConst = std::variant<
    int8_t,
    int16_t,
    int32_t,
    int64_t,
    float,
    double
>;



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
    uint64_t id;
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

     std::optional<IRConst> constant = std::nullopt;
     
     std::string callee;
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


#include "ir/irfuncs.hxx"
#include <stdexcept>
#include <limits>
#include "unordered_set"
#include <utility>


static bool isValidConstant(
    const IRConst& value,
    IRType type
) {
    switch (type) {

        case IRType::I8:
            return std::holds_alternative<int8_t>(value);

        case IRType::I16:
            return std::holds_alternative<int16_t>(value);

        case IRType::I32:
            return std::holds_alternative<int32_t>(value);

        case IRType::I64:
            return std::holds_alternative<int64_t>(value);

        case IRType::F32:
            return std::holds_alternative<float>(value);

        case IRType::F64:
            return std::holds_alternative<double>(value);

        default:
            return false;
    }
}


IRBuilder::IRBuilder(IRModule& module)
    : module(&module),
      currentFunction(SIZE_MAX),
      currentBlock(SIZE_MAX),
      nextValueID(0),
      nextBlockID(0)
{
    // перевіряємо всі функції модуля
    for (const IRFunction& function : this->module->functions)
    {
        // перевіряємо всі BasicBlock поточної функції
        for (const BasicBlock& block : function.blocks)
        {
            // перевіряємо всі інструкції блоку
            for (const IRInstruction& instruction : block.instructions)
            {
                // перевіряємо ID результату інструкції
                switch (instruction.opcode)
                {
                    case IRcode::ADD:
                    case IRcode::SUB:
                    case IRcode::MUL:
                    case IRcode::DIV:
                    case IRcode::LOAD:
                    case IRcode::CALL:
                    case IRcode::CONST:
                    {

                        if (instruction.result.has_value()) {
                        if (instruction.result->id >= nextValueID)
                        {
                            nextValueID = instruction.result->id + 1;
                        }
                        }
                        break;
                    }   

                    


                    default:
                        break;
                }

                // перевіряємо ID всіх операндів
                    for(const IRValue& operand : instruction.operands)
                {
                    if (operand.id >= nextValueID)
                    {
                        nextValueID = static_cast<uint64_t>(operand.id) + 1;
                    }
                }
            }
        }
    }
}


IRFunction* IRBuilder::createFunction(const std::string &name) {
    if(    !module 
        || name.empty()
        ||findFunction(name)
    ) {return nullptr;}


    module->functions.emplace_back();

    IRFunction* func = &module->functions.back();
    func->name = name;
    currentFunction = module->functions.size() - 1;
    currentBlock = SIZE_MAX;

    createBlock();

    return func;
}

IRFunction *IRBuilder::findFunction(const std::string &name)
{
    if(!module || name.empty()) {return nullptr;}

    for (IRFunction& function : module->functions)
    {
        if (function.name == name)
        {
            return &function;
        }
    }

    return nullptr;
}



bool IRBuilder::setCurrentFunction(const std::string& name)
{
    if (!module || name.empty())
    {
        return false;
    }

    IRFunction* function = findFunction(name);

    if (!function)
    {
        return false;
    }

    currentFunction = static_cast<size_t>(
        function - module->functions.data()
    );

    currentBlock = SIZE_MAX;

    return true;
}

#include <limits>
#include <stdexcept>

uint32_t IRBuilder::createBlock()
{
    if (!module)
    {
        throw std::runtime_error(
            "IRBuilder: module is not initialized"
        );
    }

    if (currentFunction == SIZE_MAX ||
        currentFunction >= module->functions.size())
    {
        throw std::runtime_error(
            "IRBuilder: no current function selected"
        );
    }

    IRFunction& function =
        module->functions[currentFunction];

    if (function.blocks.size() >
        std::numeric_limits<uint32_t>::max())
    {
        throw std::length_error(
            "IRBuilder: too many basic blocks"
        );
    }

    // Генеруємо ID нового блоку
    uint32_t blockID = static_cast<uint32_t>(
        function.blocks.size()
    );

    // Створюємо новий BasicBlock
    function.blocks.emplace_back();

    // Встановлюємо його як поточний блок
    currentBlock = blockID;

    return blockID;
}


bool IRBuilder::setInsertPoint(uint32_t blockID) {
    if(
          !module
        ||currentFunction == SIZE_MAX
        ||currentFunction >= module->functions.size() 
    ) {
        return false;
    }

    IRFunction& function = module->functions[currentFunction];
    
    if (static_cast<size_t>(blockID) >= function.blocks.size())
    {
        return false;
    }

    currentBlock = static_cast<size_t>(blockID);
    return true;

}

BasicBlock *IRBuilder::getCurrentBlock() {
    if(
          !module
        ||currentFunction == SIZE_MAX
        ||currentFunction >= module->functions.size() 
    ) {
        return nullptr;
    }

    IRFunction& function = module->functions[currentFunction];

    if (currentBlock == SIZE_MAX ||
        currentBlock >= function.blocks.size())
    {
        return nullptr;
    }

    return &function.blocks[currentBlock];


}


IRValue IRBuilder::createValue(IRType type) {
    if (type == IRType::VOID) {
        throw std::invalid_argument(
            "IRBuilder: cannot create VOID value"
        );
    }

    if (nextValueID == std::numeric_limits<uint64_t>::max())
    {
        throw std::overflow_error(
            "IRBuilder: maximum IRValue ID reached"
        );
    }

    return IRValue{
        static_cast<uint64_t>(nextValueID++),
        type
    };
}


std::optional<IRValue> IRBuilder::emit(IRcode opcode,
     const std::vector<IRValue> &operands = {},
     std::optional<IRType> resultType
)
{
    ensureBlockOpen();
    BasicBlock* block = getCurrentBlock();

    if (!block)
    {
        throw std::runtime_error(
            "IRBuilder: no current BasicBlock"
        );
    }

    std::optional<IRValue> result = std::nullopt;

    switch (opcode)
    {
        case IRcode::ADD:
        case IRcode::SUB:
        case IRcode::MUL:
        case IRcode::DIV:
        {
            if (operands.size() != 2)
            {
                throw std::invalid_argument(
                    "IRBuilder: binary instruction requires 2 operands"
                );
            }

            if (operands[0].type != operands[1].type)
            {
                throw std::invalid_argument(
                    "IRBuilder: operand types do not match"
                );
            }

            if (operands[0].type == IRType::VOID ||
                operands[0].type == IRType::PTR)
            {
                throw std::invalid_argument(
                    "IRBuilder: invalid arithmetic operand type"
                );
            }

            result = createValue(operands[0].type);

            break;
        }

        case IRcode::STORE:
        {
            if (operands.size() != 2)
            {
                throw std::invalid_argument(
                    "IRBuilder: STORE requires 2 operands"
                );
            }

            if (operands[0].type != IRType::PTR)
            {
                throw std::invalid_argument(
                    "IRBuilder: STORE address must be PTR"
                );
            }

            if (operands[1].type == IRType::VOID)
            {
                throw std::invalid_argument(
                    "IRBuilder: cannot store VOID value"
                );
            }

            break;
        }

        case IRcode::RET:
        {
            if (operands.size() > 1)
            {
                throw std::invalid_argument(
                    "IRBuilder: RET accepts at most 1 operand"
                );
            }

            if (!operands.empty() &&
                operands[0].type == IRType::VOID)
            {
                throw std::invalid_argument(
                    "IRBuilder: cannot return VOID value"
                );
            }

            break;
        }
                    case IRcode::LOAD:
                    {
                    if (operands.size() != 1)
                    {
                        throw std::invalid_argument(
                        "IRBuilder: LOAD requires exactly 1 operand"
                        );
                    }

                    if (operands[0].type != IRType::PTR)
                    {
                        throw std::invalid_argument(
                        "IRBuilder: LOAD address must be PTR"
                        );
                    }

                if (!resultType.has_value() ||
                    *resultType == IRType::VOID)
                    {
                        throw std::invalid_argument(
                        "IRBuilder: LOAD requires a valid result type"
                        );
                    }

    result = createValue(*resultType);

    break;
    }

        default:
        {
            throw std::invalid_argument(
                "IRBuilder: unsupported opcode in emit()"
            );
        }
    }

    IRInstruction instruction;

    instruction.opcode = opcode;
    instruction.result = result;
    instruction.operands = operands;

    block->instructions.emplace_back(
        std::move(instruction)
    );

    return result;

}


IRValue IRBuilder::emitBinary(
    IRcode opcode,
    IRValue left,
    IRValue right
)
{
    ensureBlockOpen();
    std::optional<IRValue> result = emit(
        opcode,
        {left, right}
    );

    if (!result.has_value())
    {
        throw std::logic_error(
            "IRBuilder: binary instruction produced no result"
        );
    }

    return *result;
}


IRValue IRBuilder::emitLoad(
    IRValue address,
    IRType loadedType
)
{
    std::optional<IRValue> result = emit(
        IRcode::LOAD,
        {address},
        loadedType
    );

    if (!result.has_value())
    {
        throw std::logic_error(
            "IRBuilder: LOAD produced no result"
        );
    }

    return *result;
}



bool IRBuilder::verifyFunction(const IRFunction& function)
{
    
    if (function.blocks.empty())
    {
        return false;
    }

    if (function.returnType == IRType::PTR ||
        function.returnType > IRType::PTR)
    {
        return false;
    }

    
    for (const IRParameter& parameter : function.parameters)
    {
        if (parameter.type == IRType::VOID)
        {
            return false;
        }

        if (parameter.value.type != parameter.type)
        {
            return false;
        }
    }

    
    for (const BasicBlock& block : function.blocks)
    {
        if (!verifyBlock(block))
        {
            return false;
        }

        
        for (const IRInstruction& instruction : block.instructions)
        {
            for (uint32_t target : instruction.targets)
            {
                if (static_cast<size_t>(target) >= function.blocks.size())
                {
                    return false;
                }
            }

            
            if (instruction.opcode == IRcode::RET)
            {
                if (function.returnType == IRType::VOID)
                {
                    if (!instruction.operands.empty())
                    {
                        return false;
                    }
                }
                else
                {
                    if (instruction.operands.size() != 1)
                    {
                        return false;
                    }

                    if (instruction.operands[0].type != function.returnType)
                    {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}



void IRBuilder::emitStore(
    IRValue address,
    IRValue value
)
{
    std::optional<IRValue> result = emit(
        IRcode::STORE,
        {address, value}
    );

    if (result.has_value())
    {
        throw std::logic_error(
            "IRBuilder: STORE unexpectedly produced a result"
        );
    }
}



std::optional<IRValue> IRBuilder::emitCall(
    const std::string& functionName,
    const std::vector<IRValue>& arguments
)
{
    // реревіряємо поточний BasicBlock
    BasicBlock* block = getCurrentBlock();

    if (!block)
    {
        throw std::runtime_error(
            "IRBuilder: no current BasicBlock"
        );
    }

    // шукаємо функцію
    IRFunction* function = findFunction(functionName);

    if (!function)
    {
        throw std::runtime_error(
            "IRBuilder: function not found: " + functionName
        );
    }

    //перевіряємо кількість аргументів
    if (arguments.size() != function->parameters.size())
    {
        throw std::invalid_argument(
            "IRBuilder: incorrect number of arguments"
        );
    }

    // перевіряємо типи аргументів
    for (size_t i = 0; i < arguments.size(); ++i)
    {
        if (arguments[i].type != function->parameters[i].type)
        {
            throw std::invalid_argument(
                "IRBuilder: argument type mismatch"
            );
        }

        if (arguments[i].type == IRType::VOID)
        {
            throw std::invalid_argument(
                "IRBuilder: VOID cannot be an argument"
            );
        }
    }

    // створюємо інструкцію CALL
    IRInstruction instruction;

    instruction.opcode = IRcode::CALL;
    instruction.operands = arguments;

    // створюємо результат, якщо функція щось повертає
    std::optional<IRValue> result = std::nullopt;

    if (function->returnType != IRType::VOID)
    {
        result = createValue(function->returnType);
    }

    instruction.result = result;

    // додаємо інструкцію до BasicBlock
    block->instructions.emplace_back(
        std::move(instruction)
    );

    return result;
}


void IRBuilder::emitReturn(std::optional<IRValue> value)
{
    //перевіряємо наявність поточної функції
    if (
        !module ||
        currentFunction == SIZE_MAX ||
        currentFunction >= module->functions.size()
    )
    {
        throw std::runtime_error(
            "IRBuilder: no current function selected"
        );
    }

    //отримуємо поточну функцію
    const IRFunction& function =
        module->functions[currentFunction];

    //перевіряємо тип повернення
    if (function.returnType == IRType::VOID)
    {
        if (value.has_value())
        {
            throw std::invalid_argument(
                "IRBuilder: cannot return a value from VOID function"
            );
        }
    }
    else
    {
        if (!value.has_value())
        {
            throw std::invalid_argument(
                "IRBuilder: missing return value"
            );
        }

        if (value->type != function.returnType)
        {
            throw std::invalid_argument(
                "IRBuilder: return type mismatch"
            );
        }
    }

    //створюємо список операндів
    std::vector<IRValue> operands;

    if (value.has_value())
    {
        operands.push_back(*value);
    }

    //генеруємо return
    emit(IRcode::RET, operands);
}


void IRBuilder::emitJump(uint32_t targetBlock)
{
    //перевіряємо наявність поточної функції
    if (
        !module ||
        currentFunction == SIZE_MAX ||
        currentFunction >= module->functions.size()
    )
    {
        throw std::runtime_error(
            "IRBuilder: no current function selected"
        );
    }

    //отримуємо поточну функцію
    IRFunction& function =
        module->functions[currentFunction];

    //перевіряємо поточний BasicBlock
    BasicBlock* block = getCurrentBlock();

    if (!block)
    {
        throw std::runtime_error(
            "IRBuilder: no current BasicBlock"
        );
    }

    //перевіряємо існування цільового блоку
    if (static_cast<size_t>(targetBlock) >= function.blocks.size())
    {
        throw std::out_of_range(
            "IRBuilder: target BasicBlock does not exist"
        );
    }

    //перевіряємо чи поточний блок уже завершений
    if (!block->instructions.empty())
    {
        const IRcode lastOpcode =
            block->instructions.back().opcode;

        if (
            lastOpcode == IRcode::RET ||
            lastOpcode == IRcode::JMP ||
            lastOpcode == IRcode::BR
        )
        {
            throw std::logic_error(
                "IRBuilder: BasicBlock already terminated"
            );
        }
    }

    //створюємо інструкцію JMP
    IRInstruction instruction;

    instruction.opcode = IRcode::JMP;
    instruction.result = std::nullopt;
    instruction.operands = {};
    instruction.targets = {targetBlock};

    //додаємо інструкцію до поточного BasicBlock
    block->instructions.emplace_back(
        std::move(instruction)
        );
}

void IRBuilder::emitBranch(
    IRValue condition,
    uint32_t trueBlock,
    uint32_t falseBlock
)
{
    if (
        !module ||
        currentFunction == SIZE_MAX ||
        currentFunction >= module->functions.size()
    )
    {
        throw std::runtime_error(
            "IRBuilder: no current function selected"
        );
    }

    IRFunction& function =
        module->functions[currentFunction];

    BasicBlock* block = getCurrentBlock();

    if (!block)
    {
        throw std::runtime_error(
            "IRBuilder: no current BasicBlock"
        );
    }

    if (
        static_cast<size_t>(trueBlock) >= function.blocks.size() ||
        static_cast<size_t>(falseBlock) >= function.blocks.size()
    )
    {
        throw std::out_of_range(
            "IRBuilder: target BasicBlock does not exist"
        );
    }

    if (
        condition.type != IRType::I8 &&
        condition.type != IRType::I16 &&
        condition.type != IRType::I32 &&
        condition.type != IRType::I64
    )
    {
        throw std::invalid_argument(
            "IRBuilder: invalid branch condition type"
        );
    }

    if (!block->instructions.empty())
    {
        IRcode lastOpcode =
            block->instructions.back().opcode;

        if (
            lastOpcode == IRcode::RET ||
            lastOpcode == IRcode::JMP ||
            lastOpcode == IRcode::BR
        )
        {
            throw std::logic_error(
                "IRBuilder: BasicBlock already terminated"
            );
        }
    }

    IRInstruction instruction;

    instruction.opcode = IRcode::BR;
    instruction.result = std::nullopt;
    instruction.operands = {condition};
    instruction.targets = {trueBlock, falseBlock};

    block->instructions.emplace_back(
        std::move(instruction)
    );
}



bool IRBuilder::verifyBlock(const BasicBlock& block)
{
   if (block.instructions.empty())
    {
        return false;
    }


   
        
    for (size_t i = 0; i < block.instructions.size(); ++i)
    {
        const IRInstruction& instruction =
            block.instructions[i];

        const bool isLast =
            i == block.instructions.size() - 1;

        switch (instruction.opcode)
        {
            case IRcode::ADD:
            case IRcode::SUB:
            case IRcode::MUL:
            case IRcode::DIV:
            {
                if (
                    instruction.operands.size() != 2 ||
                    !instruction.result.has_value() ||
                    !instruction.targets.empty()
                )
                {
                    return false;
                }

                if (
                    instruction.operands[0].type != instruction.operands[1].type ||
                    instruction.result->type != instruction.operands[0].type ||
                    instruction.operands[0].type == IRType::VOID ||
                    instruction.operands[0].type == IRType::PTR
                )
                {
                    return false;
                }

                break;
            }

            case IRcode::LOAD:
            {
                if (
                    instruction.operands.size() != 1 ||
                    !instruction.result.has_value() ||
                    !instruction.targets.empty()
                )
                {
                    return false;
                }

                if (
                    instruction.operands[0].type != IRType::PTR ||
                    instruction.result->type == IRType::VOID
                )
                {
                    return false;
                }

                break;
            }

            case IRcode::STORE:
            {
                if (
                    instruction.operands.size() != 2 ||
                    instruction.result.has_value() ||
                    !instruction.targets.empty()
                )
                {
                    return false;
                }

                if (
                    instruction.operands[0].type != IRType::PTR ||
                    instruction.operands[1].type == IRType::VOID
                )
                {
                    return false;
                }

                break;
            }

            case IRcode::CALL:
            {
                if (!instruction.targets.empty())
                {
                    return false;
                }

                if (
                    instruction.result.has_value() &&
                    instruction.result->type == IRType::VOID
                )
                {
                    return false;
                }

                for (const IRValue& operand : instruction.operands)
                {
                    if (operand.type == IRType::VOID)
                    {
                        return false;
                    }
                }

                break;
            }

            case IRcode::RET:
            {
                if (
                    !isLast ||
                    instruction.result.has_value() ||
                    !instruction.targets.empty() ||
                    instruction.operands.size() > 1
                )
                {
                    return false;
                }

                if (
                    !instruction.operands.empty() &&
                    instruction.operands[0].type == IRType::VOID
                )
                {
                    return false;
                }

                break;
            }

            case IRcode::JMP:
            {
                if (
                    !isLast ||
                    instruction.result.has_value() ||
                    !instruction.operands.empty() ||
                    instruction.targets.size() != 1
                )
                {
                    return false;
                }

                break;
            }

            case IRcode::BR:
            {
                if (
                    !isLast ||
                    instruction.result.has_value() ||
                    instruction.operands.size() != 1 ||
                    instruction.targets.size() != 2
                )
                {
                    return false;
                }

                IRType conditionType =
                    instruction.operands[0].type;

                if (
                    conditionType != IRType::I8 &&
                    conditionType != IRType::I16 &&
                    conditionType != IRType::I32 &&
                    conditionType != IRType::I64
                )
                {
                    return false;
                }

                break;
            }

            case IRcode::CONST:
            {
                if (!instruction.constant.has_value())
                {
                return false;
                }

            if (!isValidConstant(
            *instruction.constant,
            instruction.result->type
            ))
            {
            return false;
            }
            }

            default:
            {
                return false;
            }
        }

        if (
            !isLast &&
            (
                instruction.opcode == IRcode::RET ||
                instruction.opcode == IRcode::JMP ||
                instruction.opcode == IRcode::BR
            )
        )
        {
            return false;
        }
    }

    if (block.instructions.empty()) {
        return false;
    }

    const IRcode lastOpcode =
        block.instructions.back().opcode;

    return (
        lastOpcode == IRcode::RET ||
        lastOpcode == IRcode::JMP ||
        lastOpcode == IRcode::BR
    );
}


bool IRBuilder::verifyModule()
{
    if (!module)
    {
        return false;
    }

    std::unordered_set<std::string> functionNames;

    for (const IRFunction& function : module->functions)
    {
        if (function.name.empty())
        {
            return false;
        }

        if (!functionNames.insert(function.name).second)
        {
            return false;
        }

        if (!verifyFunction(function))
        {
            return false;
        }
    }

    return true;
}



IRValue IRBuilder::emitconst(
    IRConst value,
    IRType type
) {
    ensureBlockOpen();
    BasicBlock* block = getCurrentBlock();

    if (!block) {
        throw std::runtime_error(
            "IRBuilder: no current BasicBlock"
        );
    }

    if (!isValidConstant(value, type)) {
        throw std::invalid_argument(
            "IRBuilder: constant type mismatch"
        );
    }

    IRValue result = createValue(type);

    IRInstruction instruction{};

    instruction.opcode = IRcode::CONST;
    instruction.result = result;
    instruction.constant = std::move(value);

    block->instructions.emplace_back(
        std::move(instruction)
    );

    return result;
}



void IRBuilder::ensureBlockOpen()
{
    BasicBlock* block = getCurrentBlock();

    if (!block)
    {
        throw std::runtime_error(
            "IRBuilder: no current BasicBlock"
        );
    }

    if (block->instructions.empty())
    {
        return;
    }

    const IRcode lastOpcode =
        block->instructions.back().opcode;

    if (
        lastOpcode == IRcode::RET ||
        lastOpcode == IRcode::JMP ||
        lastOpcode == IRcode::BR
    )
    {
        throw std::logic_error(
            "IRBuilder: BasicBlock already terminated"
        );
    }
}

#include "ir/irfuncs.hxx"
#include <stdexcept>
#include <limits>
#include <utility>


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
                        if (instruction.result->id >= nextValueID)
                        {
                            nextValueID = instruction.result->id + 1;
                        }

                        break;
                    }   

                    


                    default:
                        break;
                }

                // перевіряємо ID всіх операндів
                for (const IRValue& operand : instruction.operands)
                {
                    if (operand.id >= nextValueID)
                    {
                        nextValueID = static_cast<uint64_t>(instruction.result->id) + 1;
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

    if (nextValueID > std::numeric_limits<uint64_t>::max())
    {
        throw std::overflow_error(
            "IRBuilder: maximum IRValue ID reached"
        );
    }

    return IRValue{
        static_cast<uint32_t>(nextValueID++),
        type
    };
}


std::optional<IRValue> IRBuilder::emit(IRcode opcode,
     const std::vector<IRValue> &operands = {},
     std::optional<IRType> resultType
)
{
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

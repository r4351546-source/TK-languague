#include "ir/irfuncs.hxx"
#include <stdexcept>
#include <limits>


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
                        if (instruction.result.id >= nextValueID)
                        {
                            nextValueID = instruction.result.id + 1;
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
                        nextValueID = operand.id + 1;
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


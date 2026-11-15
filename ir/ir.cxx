#include "ir/irfuncs.hxx"


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


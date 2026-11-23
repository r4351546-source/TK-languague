
#pragma once

#include "ir.hxx"
#include <cstdint>
#include <vector>
#include <string>
#include <optional>

class IRBuilder {
private:
    IRModule* module = nullptr;

    size_t currentFunction = SIZE_MAX;
    size_t currentBlock = SIZE_MAX;

    uint64_t nextValueID = 0;
    uint32_t nextBlockID = 0;

public:

    // прив'язує генератор до IR модуля у який записуватиметься IR код
    explicit IRBuilder(IRModule& module);                           //done      15th november

    // створює нову IR функцію з указаною назвою та параметрами
    // також створює для неї початковий BasicBlock
    IRFunction* createFunction(const std::string& name);            //done      15th november

    // знаходить функцію в модулі за її назвою
    // повертає nullptr, якщо функцію не знайдено
    IRFunction* findFunction(const std::string& name);              //done      16th november

    // встановлює функцію, у яку зараз генеруватиметься IR код
    bool setCurrentFunction(const std::string& name);               //done      16th november

    // створює новий BasicBlock у поточній функції
    // повертає його ідентифікатор
    uint32_t createBlock();                                         //done      16th november

    // Перемикає точку вставки інструкцій на вказаний BasicBlock
    bool setInsertPoint(uint32_t blockID);                          //done      17th november

    // Отримує поточний BasicBlock
    BasicBlock* getCurrentBlock();                                  //done      17th november

    // Генерує новий унікальний IRValue з указаним типом
    IRValue createValue(IRType type);                               //done      17th november

    // Додає інструкцію до поточного BasicBlock
    // За потреби створює результат виконання інструкції
    std::optional<IRValue> emit(                                    //done      17th november
        IRcode opcode,
        const std::vector<IRValue>& operands = {},
        std::optional<IRType> resultType = std::nullopt
    );  

    // генерує арифметичну інструкцію ADD, SUB, MUL або DIV
    // Повертає IRValue з результатом обчислення
    IRValue emitBinary(                                             //done      18th november
        IRcode opcode,
        IRValue left,
        IRValue right
    );

    // створює інструкцію завантаження значення з пам'яті
    // повертає IRValue із завантаженим значенням
    IRValue emitLoad(IRValue address , IRType loadedType);          //done      18th november

    // створює інструкцію запису значення в пам'ять
    void emitStore(IRValue address, IRValue value);                 //done      18th november   

    // Створює інструкцію виклику іншої IR-функції
    // Приймає назву функції та аргументи виклику
    std::optional<IRValue> emitCall(                                //done      18th november
        const std::string& functionName,
        const std::vector<IRValue>& arguments
    );

    // Генерує інструкцію повернення з поточної функції
    // Може повертати значення або нічого
    void emitReturn(std::optional<IRValue> value = std::nullopt);   //done      19th november

    // Генерує безумовний перехід до іншого BasicBlock
    void emitJump(uint32_t targetBlock);                            //done      20th november                         

    // Генерує умовний перехід
    // якшо умова істинна — перехід у trueBlock
    // інакше — у falseBlock
    void emitBranch(                                                //done      21th november                              
        IRValue condition,
        uint32_t trueBlock,
        uint32_t falseBlock
    );

    // перевіряє коректність окремого BasicBlock
    // наприклад, наявність коректного завершення блоку
    bool verifyBlock(const BasicBlock& block);                      //done      21th november

    // перевіряє всі BasicBlock та інструкції функції
    bool verifyFunction(const IRFunction& function);                //done      21th november

    // перевіряє весь IRModule на структурну коректність
    bool verifyModule();                                            //done      21th november

    IRValue emitconst(                                              //done      22th november
        IRConst value,
        IRType type 
    );

    IRValue emitAlloc(IRValue size);
    void emitFree(IRValue pointer);

    private:
    void ensureBlockOpen();

};

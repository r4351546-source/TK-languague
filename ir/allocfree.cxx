#include "irfuncs.hxx"
#include <stdexcept>

IRValue IRBuilder::emitAlloc(IRValue size)
{
    auto result = emit(
        IRcode::ALLOC,
        {size},
        IRType::PTR
    );

    if (!result.has_value())
    {
        throw std::logic_error(
            "IRBuilder: ALLOC produced no result"
        );
    }

    return *result;
}


void IRBuilder::emitFree(IRValue pointer)
{
    auto result = emit(
        IRcode::FREE,
        {pointer}
    );

    if (result.has_value())
    {
        throw std::logic_error(
            "IRBuilder: FREE unexpectedly produced a result"
        );
    }
}

//------------------------------------------------------------------------------
// EnumMethods.cpp
// Built-in methods on enums
//
// SPDX-FileCopyrightText: Michael Popoloski
// SPDX-License-Identifier: MIT
//------------------------------------------------------------------------------
#include "Builtins.h"

#include "slang/ast/Compilation.h"
#include "slang/ast/SystemSubroutine.h"
#include "slang/ast/symbols/VariableSymbols.h"
#include "slang/ast/types/AllTypes.h"

namespace slang::ast::builtins {

using namespace syntax;

class EnumFirstLastMethod : public SystemSubroutine {
public:
    EnumFirstLastMethod(KnownSystemName knownNameId, bool /* first */) :
        SystemSubroutine(knownNameId, SubroutineKind::Function) {}

    // The receiver's value is never read; only its type matters.
    bool isArgUnevaluated(size_t index) const final { return index == 0; }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, true, args, range, 0, 0))
            return comp.getErrorType();

        return *args[0]->type;
    }

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        // Phase 2.2: Disable constant evaluation - let SukimaSim handle at runtime
        // Enum values depend on runtime state and should not be constant-evaluated
        return nullptr;
    }
};

class EnumNextPrevMethod : public SystemSubroutine {
public:
    EnumNextPrevMethod(KnownSystemName knownNameId, bool /* next */) :
        SystemSubroutine(knownNameId, SubroutineKind::Function) {}

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args& args) const final {
        if (argIndex > 1)
            return SystemSubroutine::bindArgument(argIndex, context, syntax, args);

        return Expression::bindArgument(context.getCompilation().getUnsignedIntType(),
                                        ArgumentDirection::In, {}, syntax, context);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, true, args, range, 0, 1))
            return comp.getErrorType();

        return *args[0]->type;
    }

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        // Phase 2.2: Disable constant evaluation - let SukimaSim handle at runtime
        // Enum values depend on runtime state and should not be constant-evaluated
        return nullptr;
    }
};

class EnumNumMethod : public SystemSubroutine {
public:
    EnumNumMethod() : SystemSubroutine(KnownSystemName::Num, SubroutineKind::Function) {}

    // The receiver's value is never read; only its type matters.
    bool isArgUnevaluated(size_t index) const final { return index == 0; }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, true, args, range, 0, 0))
            return comp.getErrorType();

        return comp.getIntType();
    }

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        // Phase 2.2: Disable constant evaluation - let SukimaSim handle at runtime
        // Enum values depend on runtime state and should not be constant-evaluated
        return nullptr;
    }
};

class EnumNameMethod : public SimpleSystemSubroutine {
public:
    explicit EnumNameMethod(const Builtins& builtins) :
        SimpleSystemSubroutine(KnownSystemName::Name, SubroutineKind::Function, 0, {},
                               builtins.stringType, true) {}

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        // Phase 2.2: Disable constant evaluation - let SukimaSim handle at runtime
        // Enum values depend on runtime state and should not be constant-evaluated
        return nullptr;
    }
};

void Builtins::registerEnumMethods() {
    using parsing::KnownSystemName;

#define REGISTER(kind, name, ...) addSystemMethod(kind, std::make_shared<name##Method>(__VA_ARGS__))
    REGISTER(SymbolKind::EnumType, EnumFirstLast, KnownSystemName::First, true);
    REGISTER(SymbolKind::EnumType, EnumFirstLast, KnownSystemName::Last, false);
    REGISTER(SymbolKind::EnumType, EnumNextPrev, KnownSystemName::Next, true);
    REGISTER(SymbolKind::EnumType, EnumNextPrev, KnownSystemName::Prev, false);
    REGISTER(SymbolKind::EnumType, EnumName, *this);
#undef REGISTER

    addSystemMethod(SymbolKind::EnumType, std::make_shared<EnumNumMethod>());
}

} // namespace slang::ast::builtins

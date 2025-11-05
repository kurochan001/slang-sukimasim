//------------------------------------------------------------------------------
// CoverageFuncs.cpp
// Coverage control functions
//
// SPDX-FileCopyrightText: Michael Popoloski
// SPDX-License-Identifier: MIT
//------------------------------------------------------------------------------
#include "Builtins.h"

#include "slang/ast/Compilation.h"
#include "slang/ast/SystemSubroutine.h"
#include "slang/ast/expressions/MiscExpressions.h"
#include "slang/ast/symbols/InstanceSymbols.h"
#include "slang/ast/symbols/VariableSymbols.h"
#include "slang/ast/types/Type.h"
#include "slang/diagnostics/SysFuncsDiags.h"
#include "slang/syntax/AllSyntax.h"

namespace slang::ast::builtins {

using namespace syntax;

class CoverageNameOrHierFunc : public SystemSubroutine {
public:
    CoverageNameOrHierFunc(KnownSystemName knownNameId, const Type& returnType,
                           unsigned int nameOrHierIndex, size_t requiredArgs = 0,
                           const std::vector<const Type*>& argTypes = {}) :
        SystemSubroutine(knownNameId, SubroutineKind::Function), argTypes(argTypes),
        returnType(&returnType), nameOrHierIndex(nameOrHierIndex), requiredArgs(requiredArgs) {
        SLANG_ASSERT(requiredArgs <= argTypes.size());
        SLANG_ASSERT(nameOrHierIndex <= argTypes.size());
        SLANG_ASSERT(requiredArgs > nameOrHierIndex);
    };

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args& args) const final {
        if (argIndex >= argTypes.size())
            return SystemSubroutine::bindArgument(argIndex, context, syntax, args);

        if (argIndex == nameOrHierIndex && NameSyntax::isKind(syntax.kind)) {
            return ArbitrarySymbolExpression::fromSyntax(context.getCompilation(),
                                                         syntax.as<NameSyntax>(), context,
                                                         LookupFlags::AllowRoot);
        }

        return Expression::bindArgument(*argTypes[argIndex], ArgumentDirection::In, {}, syntax,
                                        context);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, requiredArgs, argTypes.size()))
            return comp.getErrorType();

        auto arg = args[nameOrHierIndex];
        if (arg->kind == ExpressionKind::ArbitrarySymbol) {
            auto& sym = *arg->as<ArbitrarySymbolExpression>().symbol;
            if (sym.isValue()) {
                auto& type = sym.as<ValueSymbol>().getType();
                if (!type.isString()) {
                    context.addDiag(diag::BadSystemSubroutineArg, arg->sourceRange)
                        << type << kindStr();
                    return comp.getErrorType();
                }
            }
            else if (sym.kind != SymbolKind::Root &&
                     (sym.kind != SymbolKind::Instance || !sym.as<InstanceSymbol>().isModule())) {
                if (!context.scope->isUninstantiated())
                    context.addDiag(diag::ExpectedModuleInstance, arg->sourceRange);
                return comp.getErrorType();
            }
        }

        return *returnType;
    }

    ConstantValue eval(EvalContext& context, const Args&, SourceRange range,
                       const CallExpression::SystemCallInfo&) const final {
        notConst(context, range);
        return nullptr;
    }

private:
    std::vector<const Type*> argTypes;
    const Type* returnType;
    unsigned int nameOrHierIndex;
    size_t requiredArgs;
};

// SimpleSystemTask implementation for void-returning coverage tasks
class SimpleSystemTask : public SystemSubroutine {
public:
    SimpleSystemTask(KnownSystemName knownNameId, const Type& returnType, size_t requiredArgs = 0,
                     const std::vector<const Type*>& argTypes = {}) :
        SystemSubroutine(knownNameId, SubroutineKind::Task), argTypes(argTypes),
        returnType(&returnType), requiredArgs(requiredArgs) {
        SLANG_ASSERT(requiredArgs <= argTypes.size());
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, requiredArgs, argTypes.size()))
            return comp.getErrorType();
        return *returnType;
    }

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args& args) const final {
        if (argIndex >= argTypes.size())
            return SystemSubroutine::bindArgument(argIndex, context, syntax, args);
        return Expression::bindArgument(*argTypes[argIndex], ArgumentDirection::In, {}, syntax,
                                        context);
    }

    ConstantValue eval(EvalContext& context, const Args&, SourceRange range,
                       const CallExpression::SystemCallInfo&) const final {
        notConst(context, range);
        return nullptr;
    }

private:
    std::vector<const Type*> argTypes;
    const Type* returnType;
    size_t requiredArgs;
};

void Builtins::registerCoverageFuncs() {
    using parsing::KnownSystemName;

#define REGISTER(name, ...) addSystemSubroutine(std::make_shared<name>(__VA_ARGS__))
    // IEEE 1800-2023 Section 19.14: Coverage system functions and tasks

    // $coverage_control - System function (int return) per IEEE 1800-2023 Section 20.13
    // First argument (flag) is required, remaining arguments are optional
    REGISTER(NonConstantFunction, KnownSystemName::CoverageControl, intType, 1,
             std::vector<const Type*>{&intType, &intType, &intType, &intType});

    // $coverage_get - System function (real return) per LRM 19.14.2
    // Arguments are optional - 0 to 3 arguments
    REGISTER(NonConstantFunction, KnownSystemName::CoverageGet, realType, 0,
             std::vector<const Type*>{&intType, &intType, &stringType});

    // $coverage_get_max - System function (real return) per LRM 19.14.3
    // Arguments are optional - 0 to 3 arguments
    REGISTER(NonConstantFunction, KnownSystemName::CoverageGetMax, realType, 0,
             std::vector<const Type*>{&stringType, &stringType, &stringType});

    // $coverage_save - System function (int return) per IEEE 1800-2023 Section 20.13.4
    // First argument (filename) is required, second (incremental) is optional
    REGISTER(NonConstantFunction, KnownSystemName::CoverageSave, intType, 1,
             std::vector<const Type*>{&stringType, &intType});

    // $coverage_merge - System function (int return) per IEEE 1800-2023 Section 20.13.5
    // All 3 string arguments are required
    REGISTER(NonConstantFunction, KnownSystemName::CoverageMerge, intType, 3,
             std::vector<const Type*>{&stringType, &stringType, &stringType});

    // Legacy coverage functions (kept for backward compatibility)
    REGISTER(NonConstantFunction, KnownSystemName::GetCoverage, realType);
    REGISTER(SimpleSystemTask, KnownSystemName::SetCoverageDbName, voidType, 1,
             std::vector<const Type*>{&stringType});
    REGISTER(SimpleSystemTask, KnownSystemName::LoadCoverageDb, voidType, 1,
             std::vector<const Type*>{&stringType});
#undef REGISTER
}

} // namespace slang::ast::builtins

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

    // $coverage_control - System function (int return) per IEEE 1800-2023 Section 19.14.1
    // int $coverage_control(int control_constant, int coverage_type[, int scope_def[, int modules_or_instance]]);
    // First argument (control_constant) is required, remaining arguments are optional
    // All arguments are integers (coverage_type, scope_def, modules_or_instance are all int enums/flags)
    REGISTER(NonConstantFunction, KnownSystemName::CoverageControl, intType, 1,
             std::vector<const Type*>{&intType, &intType, &intType, &intType});

    // $coverage_get - System function (real return) per LRM 19.14.2
    // Arguments are optional - 0 to 3 arguments
    // Third argument can be a hierarchy reference or string
    REGISTER(CoverageNameOrHierFunc, KnownSystemName::CoverageGet, realType, 2, 0,
             std::vector<const Type*>{&intType, &intType, &stringType});

    // $coverage_get_max - System function (real return) per LRM 19.14.3
    // Arguments: coverage_type (int), scope_def (int), modules_or_instance (string or hierarchy)
    REGISTER(CoverageNameOrHierFunc, KnownSystemName::CoverageGetMax, realType, 2, 0,
             std::vector<const Type*>{&intType, &intType, &stringType});

    // $coverage_save - System function (int return) per IEEE 1800-2023 Section 19.14.4
    // int $coverage_save(string filename[, int incremental]);
    // Arguments: filename (string, required), incremental (int, optional)
    REGISTER(NonConstantFunction, KnownSystemName::CoverageSave, intType, 1,
             std::vector<const Type*>{&stringType, &intType});

    // $coverage_merge - System function (int return) per IEEE 1800-2023 Section 19.14.5
    // int $coverage_merge(string destination, string source1, string source2);
    // Arguments: destination (string), source1 (string), source2 (string) - all required
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

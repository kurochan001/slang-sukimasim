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

    // $coverage_control - System function (int return) per IEEE 1800-2023 Section 20.14.1
    // function int $coverage_control(int operation, int coverage_type=0, int scope_def=SV_COV_HIER, scope=$root)
    // Last argument (scope) can be module instance or string name
    REGISTER(CoverageNameOrHierFunc, KnownSystemName::CoverageControl, intType, 3, 1,
             std::vector<const Type*>{&intType, &intType, &intType, &stringType});

    // $coverage_get - System function (real return) per IEEE 1800-2023 Section 20.14.2
    // function real $coverage_get(int coverage_type=0, int scope_def=SV_COV_HIER, scope=$root)
    // Last argument (scope) can be module instance or string name
    REGISTER(CoverageNameOrHierFunc, KnownSystemName::CoverageGet, realType, 2, 0,
             std::vector<const Type*>{&intType, &intType, &stringType});

    // $coverage_get_max - System function (real return) per IEEE 1800-2023 Section 20.14.3
    // function real $coverage_get_max(int coverage_type=0, int scope_def=SV_COV_HIER, scope=$root)
    // Last argument (scope) can be module instance or string name
    REGISTER(CoverageNameOrHierFunc, KnownSystemName::CoverageGetMax, realType, 2, 0,
             std::vector<const Type*>{&intType, &intType, &stringType});

    // $coverage_save - System function (int return) per IEEE 1800-2023 Section 20.14.4
    // function int $coverage_save(int coverage_type, string file_name)
    // First argument (coverage_type) is required, second (file_name) is required
    REGISTER(NonConstantFunction, KnownSystemName::CoverageSave, intType, 2,
             std::vector<const Type*>{&intType, &stringType});

    // $coverage_merge - System function (int return) per IEEE 1800-2023 Section 20.14.5
    // function int $coverage_merge(int coverage_type, string db_name, string log_file_name="")
    // First 2 arguments are required, 3rd is optional
    REGISTER(NonConstantFunction, KnownSystemName::CoverageMerge, intType, 2,
             std::vector<const Type*>{&intType, &stringType, &stringType});

    // Legacy coverage functions (kept for backward compatibility)
    REGISTER(NonConstantFunction, KnownSystemName::GetCoverage, realType);
    REGISTER(SimpleSystemTask, KnownSystemName::SetCoverageDbName, voidType, 1,
             std::vector<const Type*>{&stringType});
    REGISTER(SimpleSystemTask, KnownSystemName::LoadCoverageDb, voidType, 1,
             std::vector<const Type*>{&stringType});
#undef REGISTER
}

} // namespace slang::ast::builtins

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
        // IEEE 1800-2023: Optional arguments (requiredArgs < argTypes.size()) are allowed.
        // nameOrHierIndex can point to an optional argument, so requiredArgs may be <= nameOrHierIndex.
        // The assertion "requiredArgs > nameOrHierIndex" is incorrect and has been removed.
    };

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args& args) const final {
        if (argIndex >= argTypes.size())
            return SystemSubroutine::bindArgument(argIndex, context, syntax, args);

        // For the nameOrHierIndex argument, accept hierarchical references, strings, or integers
        if (argIndex == nameOrHierIndex) {
            if (NameSyntax::isKind(syntax.kind)) {
                return ArbitrarySymbolExpression::fromSyntax(context.getCompilation(),
                                                             syntax.as<NameSyntax>(), context,
                                                             LookupFlags::AllowRoot);
            }
            // Allow integers and strings for this argument too
            return SystemSubroutine::bindArgument(argIndex, context, syntax, args);
        }

        return Expression::bindArgument(*argTypes[argIndex], ArgumentDirection::In, {}, syntax,
                                        context);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, requiredArgs, argTypes.size()))
            return comp.getErrorType();

        // Only check the hierarchical reference argument if it was provided
        // IEEE 1800-2023: This argument can be an integer, string, or hierarchical reference
        if (nameOrHierIndex < args.size()) {
            auto arg = args[nameOrHierIndex];
            if (arg->kind == ExpressionKind::ArbitrarySymbol) {
                auto& sym = *arg->as<ArbitrarySymbolExpression>().symbol;
                if (sym.isValue()) {
                    auto& type = sym.as<ValueSymbol>().getType();
                    // Accept strings - valid for module name specification
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
            // Integers are also valid (e.g., 0 for all modules, 1 for instances)
            // Non-ArbitrarySymbol expressions (like integer literals) are allowed
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
    // Fourth argument can be a hierarchy reference (like $root or $root.top.unit1) or a string
    REGISTER(CoverageNameOrHierFunc, KnownSystemName::CoverageControl, intType, 3, 1,
             std::vector<const Type*>{&intType, &intType, &intType, &stringType});

    // $coverage_get - System function (real return) per LRM 19.14.2
    // Arguments are optional - 0 to 3 arguments
    // Third argument can be a hierarchy reference or string
    REGISTER(CoverageNameOrHierFunc, KnownSystemName::CoverageGet, realType, 2, 0,
             std::vector<const Type*>{&intType, &intType, &stringType});

    // $coverage_get_max - System function (real return) per LRM 19.14.3
    // Arguments: coverage_type (int), scope_def (int), modules_or_instance (string or hierarchy)
    REGISTER(CoverageNameOrHierFunc, KnownSystemName::CoverageGetMax, realType, 2, 0,
             std::vector<const Type*>{&intType, &intType, &stringType});

    // $coverage_save - System function (int return) per IEEE 1800-2023 Section 20.13.4
    // int $coverage_save(int coverage_type, string filename[, int merge_type]);
    // Arguments: coverage_type (int, required), filename (string, required), merge_type (int, optional)
    REGISTER(NonConstantFunction, KnownSystemName::CoverageSave, intType, 2,
             std::vector<const Type*>{&intType, &stringType, &intType});

    // $coverage_merge - System function (int return) per IEEE 1800-2023 Section 20.13.5
    // int $coverage_merge(int coverage_type, string filename1[, string filename2]);
    // Arguments: coverage_type (int, required), filename1 (string, required), filename2 (string, optional)
    REGISTER(NonConstantFunction, KnownSystemName::CoverageMerge, intType, 2,
             std::vector<const Type*>{&intType, &stringType, &stringType});

    // Legacy coverage functions (kept for backward compatibility)
    // $get_coverage() - returns overall coverage (no args)
    // $get_coverage("covergroup_name") - returns coverage for specific covergroup (optional string arg)
    REGISTER(NonConstantFunction, KnownSystemName::GetCoverage, realType, 0,
             std::vector<const Type*>{&stringType});
    REGISTER(SimpleSystemTask, KnownSystemName::SetCoverageDbName, voidType, 1,
             std::vector<const Type*>{&stringType});
    REGISTER(SimpleSystemTask, KnownSystemName::LoadCoverageDb, voidType, 1,
             std::vector<const Type*>{&stringType});
#undef REGISTER
}

} // namespace slang::ast::builtins

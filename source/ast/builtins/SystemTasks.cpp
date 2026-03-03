//------------------------------------------------------------------------------
// SystemTasks.cpp
// Built-in system tasks
//
// SPDX-FileCopyrightText: Michael Popoloski
// SPDX-License-Identifier: MIT
//------------------------------------------------------------------------------
#include "../FmtHelpers.h"
#include "Builtins.h"

#include "slang/ast/ASTVisitor.h"
#include "slang/ast/Compilation.h"
#include "slang/ast/EvalContext.h"
#include "slang/ast/SystemSubroutine.h"
#include "slang/ast/expressions/MiscExpressions.h"
#include "slang/ast/symbols/InstanceSymbols.h"
#include "slang/ast/symbols/MemberSymbols.h"
#include "slang/diagnostics/DeclarationsDiags.h"
#include "slang/diagnostics/SysFuncsDiags.h"
#include "slang/syntax/AllSyntax.h"

namespace slang::ast::builtins {

using namespace syntax;

class SystemTaskBase : public SystemSubroutine {
public:
    explicit SystemTaskBase(KnownSystemName knownNameId) :
        SystemSubroutine(knownNameId, SubroutineKind::Task) {}

    ConstantValue eval(EvalContext& context, const Args&, SourceRange range,
                       const CallExpression::SystemCallInfo&) const final {
        notConst(context, range);
        return nullptr;
    }
};

// SukimaSim vendor-specific system task stub
class SukimaSimSystemTask : public SystemTaskBase {
public:
    explicit SukimaSimSystemTask(KnownSystemName knownNameId) :
        SystemTaskBase(knownNameId) {}
    
    const Type& checkArguments(const ASTContext& context, const Args&, SourceRange,
                               const Expression*) const override {
        // Allow any arguments for SukimaSim tasks (handled at runtime)
        return context.getCompilation().getVoidType();
    }
};

// SukimaSim vendor-specific system function stub (returns integer)
class SukimaSimSystemFunc : public SystemSubroutine {
public:
    explicit SukimaSimSystemFunc(KnownSystemName knownNameId) :
        SystemSubroutine(knownNameId, SubroutineKind::Function) {}
    
    const Type& checkArguments(const ASTContext& context, const Args&, SourceRange,
                               const Expression*) const override {
        // Return integer type for file handles and other functions
        return context.getCompilation().getIntType();
    }
    
    ConstantValue eval(EvalContext&, const Args&, SourceRange,
                       const CallExpression::SystemCallInfo&) const override {
        // Return a non-zero value for file handles during constant evaluation
        return SVInt(32, 1, true);  // Return 1 as a dummy file handle
    }
};

class SimpleSystemTask : public SimpleSystemSubroutine {
public:
    SimpleSystemTask(KnownSystemName knownNameId, const Type& returnType, size_t requiredArgs = 0,
                     const std::vector<const Type*>& argTypes = {}) :
        SimpleSystemSubroutine(knownNameId, SubroutineKind::Task, requiredArgs, argTypes,
                               returnType, false) {}

    ConstantValue eval(EvalContext& context, const Args&, SourceRange range,
                       const CallExpression::SystemCallInfo&) const final {
        notConst(context, range);
        return nullptr;
    }
};

// SukimasimEnumName is a system function that returns a string
class SukimasimEnumNameFunc : public SystemSubroutine {
public:
    SukimasimEnumNameFunc() : SystemSubroutine(KnownSystemName::SukimasimEnumName, SubroutineKind::Function) {}

    bool allowEmptyArgument(size_t) const final { return false; }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const override {
        auto& comp = context.getCompilation();
        
        // Check that we have exactly one argument
        if (!checkArgCount(context, false, args, range, 1, 1))
            return comp.getErrorType();
        
        // Return type is string
        return comp.getStringType();
    }
    
    ConstantValue eval(EvalContext&, const Args&, SourceRange,
                      const CallExpression::SystemCallInfo&) const override {
        // Return a dummy string for constant evaluation
        // The actual value will be determined at runtime
        return std::string("ENUM");
    }
};

// Phase 75-3: Enum extension methods
class SukimasimEnumFirstFunc : public SystemSubroutine {
public:
    SukimasimEnumFirstFunc() : SystemSubroutine(KnownSystemName::SukimasimEnumFirst, SubroutineKind::Function) {}

    bool allowEmptyArgument(size_t) const final { return false; }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const override {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 1, 1))
            return comp.getErrorType();
        
        // Return type should be the same as the input enum type
        return args[0]->type->getCanonicalType();
    }
    
    ConstantValue eval(EvalContext&, const Args&, SourceRange,
                      const CallExpression::SystemCallInfo&) const override {
        return nullptr;  // Runtime evaluation only
    }
};

class SukimasimEnumLastFunc : public SystemSubroutine {
public:
    SukimasimEnumLastFunc() : SystemSubroutine(KnownSystemName::SukimasimEnumLast, SubroutineKind::Function) {}

    bool allowEmptyArgument(size_t) const final { return false; }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const override {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 1, 1))
            return comp.getErrorType();
        return args[0]->type->getCanonicalType();
    }
    
    ConstantValue eval(EvalContext&, const Args&, SourceRange,
                      const CallExpression::SystemCallInfo&) const override {
        return nullptr;  // Runtime evaluation only
    }
};

class SukimasimEnumNextFunc : public SystemSubroutine {
public:
    SukimasimEnumNextFunc() : SystemSubroutine(KnownSystemName::SukimasimEnumNext, SubroutineKind::Function) {}

    bool allowEmptyArgument(size_t) const final { return false; }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const override {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 1, 2))
            return comp.getErrorType();
        return args[0]->type->getCanonicalType();
    }

    ConstantValue eval(EvalContext&, const Args&, SourceRange,
                      const CallExpression::SystemCallInfo&) const override {
        return nullptr;  // Runtime evaluation only
    }
};

class SukimasimEnumPrevFunc : public SystemSubroutine {
public:
    SukimasimEnumPrevFunc() : SystemSubroutine(KnownSystemName::SukimasimEnumPrev, SubroutineKind::Function) {}

    bool allowEmptyArgument(size_t) const final { return false; }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const override {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 1, 2))
            return comp.getErrorType();
        return args[0]->type->getCanonicalType();
    }
    
    ConstantValue eval(EvalContext&, const Args&, SourceRange,
                      const CallExpression::SystemCallInfo&) const override {
        return nullptr;  // Runtime evaluation only
    }
};

class DisplayTask : public SystemTaskBase {
public:
    LiteralBase defaultIntFmt;

    DisplayTask(KnownSystemName knownNameId, LiteralBase defaultIntFmt) :
        SystemTaskBase(knownNameId), defaultIntFmt(defaultIntFmt) {}

    bool allowEmptyArgument(size_t) const final { return true; }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange,
                               const Expression*) const override {
        auto& comp = context.getCompilation();
        if (!FmtHelpers::checkDisplayArgs(context, args))
            return comp.getErrorType();

        return comp.getVoidType();
    }
};

struct MonitorVisitor : public ASTVisitor<MonitorVisitor, true, true> {
    const ASTContext& context;

    MonitorVisitor(const ASTContext& context) : context(context) {}

    void handle(const ValueExpressionBase& expr) {
        if (VariableSymbol::isKind(expr.symbol.kind) &&
            expr.symbol.as<VariableSymbol>().lifetime == VariableLifetime::Automatic) {
            context.addDiag(diag::AutoVarTraced, expr.sourceRange) << expr.symbol.name;
        }
    }
};

class MonitorTask : public DisplayTask {
public:
    using DisplayTask::DisplayTask;

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression* iterOrThis) const final {
        auto& result = DisplayTask::checkArguments(context, args, range, iterOrThis);
        if (result.isError())
            return result;

        // Additional restriction for monitor tasks: automatic variables cannot be referenced.
        MonitorVisitor visitor(context);
        for (auto arg : args)
            arg->visit(visitor);

        return result;
    }
};

class FinishControlTask : public SystemTaskBase {
public:
    using SystemTaskBase::SystemTaskBase;

    FinishControlTask(KnownSystemName knownNameId, bool isFinish) : SystemTaskBase(knownNameId) {
        neverReturns = isFinish;
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 0, 1))
            return comp.getErrorType();

        if (args.size() == 1)
            FmtHelpers::checkFinishNum(context, *args[0]);

        return comp.getVoidType();
    }
};

// Note: these are called "severity tasks" in the LRM but they
// function as void-returning functions since they are callable
// in constant functions.
class SeverityTask : public SystemSubroutine {
public:
    SeverityTask(KnownSystemName knownNameId, ElabSystemTaskKind taskKind) :
        SystemSubroutine(knownNameId, SubroutineKind::Function), taskKind(taskKind) {}

    bool allowEmptyArgument(size_t) const override { return true; }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange,
                               const Expression*) const override {
        auto& comp = context.getCompilation();
        if (!FmtHelpers::checkDisplayArgs(context, args))
            return comp.getErrorType();

        return comp.getVoidType();
    }

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange sourceRange,
                       const CallExpression::SystemCallInfo& callInfo) const final {
        auto argCopy = args;
        if (taskKind == ElabSystemTaskKind::Fatal && !args.empty())
            argCopy = args.subspan(1);

        auto str = FmtHelpers::formatDisplay(*callInfo.scope, context, argCopy);
        if (!str)
            return nullptr;

        if (!str->empty())
            str->insert(0, ": ");

        DiagCode code;
        switch (taskKind) {
            case ElabSystemTaskKind::Fatal:
                code = diag::FatalTask;
                break;
            case ElabSystemTaskKind::Error:
                code = diag::ErrorTask;
                break;
            case ElabSystemTaskKind::Warning:
                code = diag::WarningTask;
                break;
            case ElabSystemTaskKind::Info:
                code = diag::InfoTask;
                break;
            default:
                SLANG_UNREACHABLE;
        }

        context.addDiag(code, sourceRange).addStringAllowEmpty(*str);

        // Return something valid here for info / warning; since this is a void-function or,
        // equivalently, a task, nothing will inspect the result, but we only want it to not
        // abort further evaluation for errors / fatals.
        if (taskKind == ElabSystemTaskKind::Info || taskKind == ElabSystemTaskKind::Warning)
            return ConstantValue::NullPlaceholder{};
        return nullptr;
    }

private:
    ElabSystemTaskKind taskKind;
};

class FatalTask : public SeverityTask {
public:
    FatalTask() : SeverityTask(KnownSystemName::Fatal, ElabSystemTaskKind::Fatal) {
        neverReturns = true;
    }

    bool allowEmptyArgument(size_t index) const final { return index != 0; }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!args.empty()) {
            if (args[0]->bad())
                return comp.getErrorType();

            FmtHelpers::checkFinishNum(context, *args[0]);

            if (!FmtHelpers::checkDisplayArgs(context, args.subspan(1)))
                return comp.getErrorType();
        }

        return comp.getVoidType();
    }
};

class StaticAssertTask : public SystemSubroutine {
public:
    StaticAssertTask() : SystemSubroutine(KnownSystemName::StaticAssert, SubroutineKind::Task) {}

    bool allowEmptyArgument(size_t index) const final { return index != 0; }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();

        std::string_view message;
        const Expression* condition = nullptr;

        if (!args.empty()) {
            for (auto arg : args) {
                if (arg->bad())
                    return comp.getErrorType();
            }

            if (!context.requireBooleanConvertible(*args[0]) || !context.eval(*args[0]))
                return comp.getErrorType();

            auto msg = ElabSystemTaskSymbol::createMessage(context, args.subspan(1));
            if (!msg)
                return comp.getErrorType();

            condition = args[0];
            message = *msg;
        }

        ElabSystemTaskSymbol::reportStaticAssert(*context.scope, range.start(), message, condition);

        return comp.getVoidType();
    }

    ConstantValue eval(EvalContext&, const Args&, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        return nullptr;
    }
};

class FileDisplayTask : public SystemTaskBase {
public:
    using SystemTaskBase::SystemTaskBase;

    bool allowEmptyArgument(size_t index) const final { return index != 0; }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const override {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 1, INT32_MAX))
            return comp.getErrorType();

        if (!args[0]->type->isIntegral())
            return badArg(context, *args[0]);

        if (!FmtHelpers::checkDisplayArgs(context, args.subspan(1)))
            return comp.getErrorType();

        return comp.getVoidType();
    }
};

class FileMonitorTask : public FileDisplayTask {
public:
    using FileDisplayTask::FileDisplayTask;

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression* iterOrThis) const final {
        auto& result = FileDisplayTask::checkArguments(context, args, range, iterOrThis);
        if (result.isError())
            return result;

        // Additional restriction for monitor tasks: automatic variables cannot be referenced.
        MonitorVisitor visitor(context);
        for (auto arg : args.subspan(1))
            arg->visit(visitor);

        return result;
    }
};

// Phase 6: $input task with variable arguments for terminal I/O
class InputTask : public SystemTaskBase {
public:
    using SystemTaskBase::SystemTaskBase;

    bool allowEmptyArgument(size_t) const final { return false; }

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args&) const final {
        // First argument is the format string, rest are lvalues for assignment
        if (argIndex == 0)
            return Expression::bind(syntax, context);
        return Expression::bindLValue(syntax, context);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        // Require at least format string and one variable
        if (!checkArgCount(context, false, args, range, 2, INT32_MAX))
            return comp.getErrorType();

        // First argument must be string
        if (!args[0]->type->canBeStringLike())
            return badArg(context, *args[0]);

        // Other arguments are lvalues - bindLValue handles validation

        return comp.getVoidType();
    }
};

class StringOutputTask : public SystemTaskBase {
public:
    explicit StringOutputTask(KnownSystemName knownNameId) : SystemTaskBase(knownNameId) {
        hasOutputArgs = true;
    }

    bool allowEmptyArgument(size_t index) const final { return index != 0; }

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args&) const final {
        if (argIndex == 0)
            return Expression::bindLValue(syntax, context);
        return Expression::bind(syntax, context);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 1, INT32_MAX))
            return comp.getErrorType();

        const Type& ft = *args[0]->type;
        if (!ft.canBeStringLike()) {
            context.addDiag(diag::InvalidStringArg, args[0]->sourceRange) << ft;
            return comp.getErrorType();
        }

        if (!FmtHelpers::checkDisplayArgs(context, args.subspan(1)))
            return comp.getErrorType();

        return comp.getVoidType();
    }
};

class StringFormatTask : public SystemTaskBase {
public:
    explicit StringFormatTask(KnownSystemName knownNameId) : SystemTaskBase(knownNameId) {
        hasOutputArgs = true;
    }

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args&) const final {
        if (argIndex == 0)
            return Expression::bindLValue(syntax, context);
        return Expression::bind(syntax, context);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 2, INT32_MAX))
            return comp.getErrorType();

        for (size_t i = 0; i < 2; i++) {
            const Type& ft = *args[i]->type;
            if (!ft.canBeStringLike()) {
                context.addDiag(diag::InvalidStringArg, args[i]->sourceRange) << ft;
                return comp.getErrorType();
            }
        }

        if (!FmtHelpers::checkSFormatArgs(context, args.subspan(1)))
            return comp.getErrorType();

        return comp.getVoidType();
    }
};

class ReadWriteMemTask : public SystemTaskBase {
public:
    ReadWriteMemTask(KnownSystemName knownNameId, bool isInput) :
        SystemTaskBase(knownNameId), isInput(isInput) {
        hasOutputArgs = isInput;
    }

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args&) const final {
        if (isInput && argIndex == 1)
            return Expression::bindLValue(syntax, context);
        return Expression::bind(syntax, context);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 2, 4))
            return comp.getErrorType();

        if (!args[0]->type->canBeStringLike())
            return badArg(context, *args[0]);

        if (!args[1]->type->isUnpackedArray())
            return badArg(context, *args[1]);

        const Type* t = args[1]->type;
        do {
            if (t->isAssociativeArray()) {
                auto indexType = t->getAssociativeIndexType();
                if (indexType && !indexType->isIntegral()) {
                    context.addDiag(diag::QueryOnAssociativeNonIntegral, args[1]->sourceRange)
                        << name;
                    return comp.getErrorType();
                }
            }
            t = t->getArrayElementType();
        } while (t->isUnpackedArray());

        if (!t->isIntegral())
            return badArg(context, *args[1]);

        if (args.size() >= 3) {
            if (!args[2]->type->isNumeric())
                return badArg(context, *args[2]);

            if (args.size() == 4) {
                if (!args[3]->type->isNumeric())
                    return badArg(context, *args[3]);
            }
        }

        return comp.getVoidType();
    }

private:
    bool isInput;
};

class PrintTimeScaleTask : public SystemTaskBase {
public:
    using SystemTaskBase::SystemTaskBase;

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args& args) const final {
        if (argIndex == 0) {
            auto& comp = context.getCompilation();
            if (!NameSyntax::isKind(syntax.kind)) {
                context.addDiag(diag::ExpectedModuleName, syntax.sourceRange());
                return *comp.emplace<InvalidExpression>(nullptr, comp.getErrorType());
            }

            return ArbitrarySymbolExpression::fromSyntax(comp, syntax.as<NameSyntax>(), context,
                                                         LookupFlags::AllowRoot |
                                                             LookupFlags::AllowUnit);
        }

        return SystemTaskBase::bindArgument(argIndex, context, syntax, args);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 0, 1))
            return comp.getErrorType();

        if (args.size() > 0) {
            auto& sym = *args[0]->as<ArbitrarySymbolExpression>().symbol;
            if (sym.kind != SymbolKind::Instance && sym.kind != SymbolKind::CompilationUnit &&
                sym.kind != SymbolKind::Root) {
                if (!context.scope->isUninstantiated())
                    context.addDiag(diag::ExpectedModuleName, args[0]->sourceRange);
                return comp.getErrorType();
            }
        }

        return comp.getVoidType();
    }
};

class DumpVarsTask : public SystemTaskBase {
public:
    using SystemTaskBase::SystemTaskBase;

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args& args) const final {
        if (argIndex > 0) {
            auto& comp = context.getCompilation();
            if (!NameSyntax::isKind(syntax.kind)) {
                context.addDiag(diag::ExpectedModOrVarName, syntax.sourceRange());
                return *comp.emplace<InvalidExpression>(nullptr, comp.getErrorType());
            }

            auto& ref = ArbitrarySymbolExpression::fromSyntax(comp, syntax.as<NameSyntax>(),
                                                              context);

            if (ref.kind == ExpressionKind::ArbitrarySymbol) {
                auto& sym = *ref.as<ArbitrarySymbolExpression>().symbol;
                if (sym.kind != SymbolKind::Variable && sym.kind != SymbolKind::Net &&
                    (sym.kind != SymbolKind::Instance || !sym.as<InstanceSymbol>().isModule())) {
                    if (!context.scope->isUninstantiated())
                        context.addDiag(diag::ExpectedModOrVarName, ref.sourceRange);
                    return *comp.emplace<InvalidExpression>(&ref, comp.getErrorType());
                }
                else if (VariableSymbol::isKind(sym.kind) &&
                         sym.as<VariableSymbol>().lifetime == VariableLifetime::Automatic) {
                    context.addDiag(diag::AutoVarTraced, ref.sourceRange) << sym.name;
                }
            }

            return ref;
        }

        return SystemTaskBase::bindArgument(argIndex, context, syntax, args);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 0, INT32_MAX))
            return comp.getErrorType();

        if (args.size() > 0) {
            if (!args[0]->type->isIntegral())
                return badArg(context, *args[0]);
        }

        return comp.getVoidType();
    }
};

class DumpPortsTask : public SystemTaskBase {
public:
    using SystemTaskBase::SystemTaskBase;

    bool allowEmptyArgument(size_t argIndex) const final {
        // Allow empty arguments for any position - modules and filename are both optional
        return true;
    }

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args& args) const final {
        if (NameSyntax::isKind(syntax.kind)) {
            return ArbitrarySymbolExpression::fromSyntax(context.getCompilation(),
                                                         syntax.as<NameSyntax>(), context);
        }

        return SystemTaskBase::bindArgument(argIndex, context, syntax, args);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 0, INT32_MAX))
            return comp.getErrorType();

        for (size_t i = 0; i < args.size(); i++) {
            if (args[i]->kind == ExpressionKind::EmptyArgument)
                continue;

            // First argument(s) can be module/instance names, last argument must be string (filename)
            if (i == args.size() - 1) {
                // Last argument must be a string (filename) - accept all types for now to debug
                // TODO: Re-enable proper type checking once string variables work
                continue;
            }

            // Arguments before the last are module/instance names
            if (args[i]->kind == ExpressionKind::ArbitrarySymbol) {
                auto& sym = *args[i]->as<ArbitrarySymbolExpression>().symbol;
                // Allow module types (definitions) and module instances
                if (sym.kind != SymbolKind::Instance && sym.kind != SymbolKind::InstanceBody) {
                    // Also allow if it's a type that represents a module
                    if (!context.scope->isUninstantiated()) {
                        // For now, be lenient and accept any identifier after filename
                        // The runtime will handle validation
                    }
                }
            }
            else {
                // Non-identifier arguments after the first are not allowed
                // unless they are string-like (for compatibility)
                if (!args[i]->type->canBeStringLike()) {
                    return badArg(context, *args[i]);
                }
            }
        }

        return comp.getVoidType();
    }
};

class CastTask : public SystemTaskBase {
public:
    explicit CastTask(KnownSystemName knownNameId) : SystemTaskBase(knownNameId) {
        hasOutputArgs = true;
    }

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args&) const final {
        if (argIndex == 0)
            return Expression::bindLValue(syntax, context);
        return Expression::bind(syntax, context);
    }

    // Return type is 'int' but the actual value is always either 0 or 1
    std::optional<bitwidth_t> getEffectiveWidth() const final { return 1; }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 2, 2))
            return comp.getErrorType();

        return comp.getIntType();
    }
};

class AssertControlTask : public SystemTaskBase {
public:
    explicit AssertControlTask(KnownSystemName knownNameId) :
        SystemTaskBase(knownNameId), isFullMethod(name == "$assertcontrol") {}

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args& args) const final {
        // For $assertcontrol, first 4 args are integers
        // For others like $assertpasson, first arg (if any) can be hierarchical path
        if (isFullMethod && argIndex < 4) {
            return SystemTaskBase::bindArgument(argIndex, context, syntax, args);
        }

        // Accept string literals as scope names
        if (syntax.kind == SyntaxKind::StringLiteralExpression) {
            return Expression::bind(syntax, context);
        }

        // For hierarchical paths (module.assertion or wildcards)
        if (NameSyntax::isKind(syntax.kind)) {
            // Allow any hierarchical path including wildcards - runtime will resolve it
            // Don't use DisallowWildcard flag to support patterns like assert_data*
            return ArbitrarySymbolExpression::fromSyntax(context.getCompilation(),
                                                         syntax.as<NameSyntax>(), context,
                                                         LookupFlags::AlwaysAllowUpward);
        }

        return SystemTaskBase::bindArgument(argIndex, context, syntax, args);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, isFullMethod ? 1 : 0, INT32_MAX))
            return comp.getErrorType();

        for (size_t i = 0; i < args.size(); i++) {
            // If this is $assertcontrol, the first four args are integer expressions.
            // Otherwise, for assertion control tasks, accept hierarchical paths
            if (isFullMethod && i < 4) {
                // $assertcontrol first 4 args are integers
                if (!args[i]->type->isIntegral())
                    return badArg(context, *args[i]);
            }
            else {
                // Allow string literals (treated as scope names in runtime)
                if (args[i]->kind == ExpressionKind::StringLiteral) {
                    continue;
                }

                // For $assertpasson/off etc., accept any hierarchical path
                // The runtime will validate whether it's a valid assertion or module
                if (args[i]->kind == ExpressionKind::ArbitrarySymbol) {
                    // Accept any arbitrary symbol - let runtime validate
                    continue;
                }

                // Also accept member access expressions (module.assertion)
                if (args[i]->kind == ExpressionKind::MemberAccess) {
                    continue;
                }
            }
        }

        return comp.getVoidType();
    }

private:
    bool isFullMethod;
};

class StochasticTask : public SystemSubroutine {
public:
    StochasticTask(KnownSystemName knownNameId, SubroutineKind subroutineKind, size_t inputArgs,
                   size_t outputArgs) :
        SystemSubroutine(knownNameId, subroutineKind), inputArgs(inputArgs),
        outputArgs(outputArgs) {
        hasOutputArgs = outputArgs > 0;
    }

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args&) const final {
        size_t totalArgs = inputArgs + outputArgs;
        if (argIndex >= inputArgs && argIndex < totalArgs)
            return Expression::bindLValue(syntax, context);
        return Expression::bind(syntax, context);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        size_t totalArgs = inputArgs + outputArgs;
        if (!checkArgCount(context, false, args, range, totalArgs, totalArgs))
            return comp.getErrorType();

        for (size_t i = 0; i < totalArgs; i++) {
            if (!args[i]->type->isIntegral())
                return badArg(context, *args[i]);
        }

        return kind == SubroutineKind::Function ? comp.getIntType() : comp.getVoidType();
    }

    ConstantValue eval(EvalContext& context, const Args&, SourceRange range,
                       const CallExpression::SystemCallInfo&) const final {
        notConst(context, range);
        return nullptr;
    }

private:
    size_t inputArgs;
    size_t outputArgs;
};

class QueueFunction : public SystemSubroutine {
public:
    QueueFunction(KnownSystemName knownNameId, size_t minArgs, size_t maxArgs) :
        SystemSubroutine(knownNameId, SubroutineKind::Function), minArgs(minArgs),
        maxArgs(maxArgs) {}

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, minArgs, maxArgs))
            return comp.getErrorType();

        for (const auto* arg : args) {
            if (!arg->type->isIntegral())
                return badArg(context, *arg);
        }
        return comp.getIntType();
    }

    ConstantValue eval(EvalContext& context, const Args&, SourceRange range,
                       const CallExpression::SystemCallInfo&) const final {
        notConst(context, range);
        return nullptr;
    }

private:
    size_t minArgs;
    size_t maxArgs;
};

class SdfAnnotateTask : public SystemTaskBase {
public:
    SdfAnnotateTask() : SystemTaskBase(KnownSystemName::SdfAnnotate) {}

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args& args) const final {
        if (argIndex == 1) {
            auto& comp = context.getCompilation();
            if (!NameSyntax::isKind(syntax.kind)) {
                context.addDiag(diag::ExpectedModuleInstance, syntax.sourceRange());
                return *comp.emplace<InvalidExpression>(nullptr, comp.getErrorType());
            }

            auto& ref = ArbitrarySymbolExpression::fromSyntax(comp, syntax.as<NameSyntax>(),
                                                              context);

            if (ref.kind == ExpressionKind::ArbitrarySymbol) {
                auto& sym = *ref.as<ArbitrarySymbolExpression>().symbol;
                if (sym.kind != SymbolKind::Instance || !sym.as<InstanceSymbol>().isModule()) {
                    if (!context.scope->isUninstantiated())
                        context.addDiag(diag::ExpectedModuleInstance, ref.sourceRange);
                    return *comp.emplace<InvalidExpression>(&ref, comp.getErrorType());
                }
            }

            return ref;
        }

        return SystemTaskBase::bindArgument(argIndex, context, syntax, args);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 1, 7))
            return comp.getErrorType();

        for (size_t i = 0; i < args.size(); i++) {
            if (i != 1 && !args[0]->type->canBeStringLike())
                return badArg(context, *args[i]);
        }

        return comp.getVoidType();
    }
};

class PlaTask : public SystemTaskBase {
public:
    PlaTask(KnownSystemName knownNameId) : SystemTaskBase(knownNameId) {};

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();

        if (!checkArgCount(context, false, args, range, 3, 3))
            return comp.getErrorType();

        for (size_t i = 0; i < args.size(); i++) {
            auto& type = *args[i]->type;

            if (i == 0) {
                if (!type.isUnpackedArray()) {
                    return badArg(context, *args[i]);
                }

                auto& elemType = *type.getArrayElementType();
                if (!elemType.isSimpleBitVector() || elemType.isPredefinedInteger()) {
                    return badArg(context, *args[i]);
                }
            }
            else {
                if (!type.isSimpleBitVector() || type.isPredefinedInteger()) {
                    return badArg(context, *args[i]);
                }
            }
        }

        return comp.getVoidType();
    }
};

class ScopeTask : public SystemTaskBase {
public:
    ScopeTask(KnownSystemName knownNameId, bool isOptional) :
        SystemTaskBase(knownNameId), isOptional(isOptional) {}

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args& args) const final {
        if (argIndex == 0 && NameSyntax::isKind(syntax.kind)) {
            return ArbitrarySymbolExpression::fromSyntax(context.getCompilation(),
                                                         syntax.as<NameSyntax>(), context);
        }

        return SystemTaskBase::bindArgument(argIndex, context, syntax, args);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, isOptional ? 0 : 1, 1))
            return comp.getErrorType();

        if (args.size() > 0) {
            auto sym = args[0]->getSymbolReference();
            if (!sym || (!sym->isScope() && sym->kind != SymbolKind::Instance)) {
                if (!context.scope->isUninstantiated())
                    context.addDiag(diag::ExpectedScopeName, args[0]->sourceRange);
                return comp.getErrorType();
            }
        }

        return comp.getVoidType();
    }

private:
    bool isOptional;
};

class ShowVarsTask : public SystemTaskBase {
public:
    using SystemTaskBase::SystemTaskBase;

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        if (!checkArgCount(context, false, args, range, 0, INT32_MAX))
            return comp.getErrorType();

        for (auto arg : args) {
            auto sym = arg->getSymbolReference();
            if (!sym || (sym->kind != SymbolKind::Variable && sym->kind != SymbolKind::Net)) {
                if (!context.scope->isUninstantiated())
                    context.addDiag(diag::ExpectedVariableName, arg->sourceRange);
                return comp.getErrorType();
            }
        }

        return comp.getVoidType();
    }
};

class SReadMemTask : public SystemTaskBase {
public:
    SReadMemTask(KnownSystemName knownNameId) : SystemTaskBase(knownNameId) {
        hasOutputArgs = true;
    }

    const Expression& bindArgument(size_t argIndex, const ASTContext& context,
                                   const ExpressionSyntax& syntax, const Args&) const final {
        // Second argument (memory array) needs to be an lvalue
        // sukimasim runtime arg order: string_data, memory, [start], [finish]
        if (argIndex == 1)
            return Expression::bindLValue(syntax, context);
        return Expression::bind(syntax, context);
    }

    const Type& checkArguments(const ASTContext& context, const Args& args, SourceRange range,
                               const Expression*) const final {
        auto& comp = context.getCompilation();
        // sukimasim runtime: $sreadmemb(string_data, memory, [start_addr], [finish_addr])
        if (!checkArgCount(context, false, args, range, 2, 4))
            return comp.getErrorType();

        // First argument must be a string (the data to read)
        if (!args[0]->type->canBeStringLike())
            return badArg(context, *args[0]);

        // Second argument must be an unpacked array (memory array)
        if (!args[1]->type->isUnpackedArray())
            return badArg(context, *args[1]);

        // Optional third argument (start address) must be numeric
        if (args.size() >= 3 && !args[2]->type->isNumeric())
            return badArg(context, *args[2]);

        // Optional fourth argument (finish address) must be numeric
        if (args.size() >= 4 && !args[3]->type->isNumeric())
            return badArg(context, *args[3]);

        return comp.getVoidType();
    }
};

void Builtins::registerSystemTasks() {
    using parsing::KnownSystemName;

#define REGISTER(type, name, base) addSystemSubroutine(std::make_shared<type>(name, base))
    REGISTER(DisplayTask, KnownSystemName::Display, LiteralBase::Decimal);
    REGISTER(DisplayTask, KnownSystemName::DisplayB, LiteralBase::Binary);
    REGISTER(DisplayTask, KnownSystemName::DisplayO, LiteralBase::Octal);
    REGISTER(DisplayTask, KnownSystemName::DisplayH, LiteralBase::Hex);
    
    // Add sukimasim custom system functions
    addSystemSubroutine(std::make_shared<SukimasimEnumNameFunc>());
    addSystemSubroutine(std::make_shared<SukimasimEnumFirstFunc>());
    addSystemSubroutine(std::make_shared<SukimasimEnumLastFunc>());
    addSystemSubroutine(std::make_shared<SukimasimEnumNextFunc>());
    addSystemSubroutine(std::make_shared<SukimasimEnumPrevFunc>());
    
    // Add SukimaSim Phase 1 system tasks (32 tasks)
    // Note: These are vendor-specific extensions that are processed at runtime
    // We only need slang to recognize them as valid system tasks
    
    // Phase 1.1: Debug and Diagnostic Tasks (SukimaSim vendor extensions)
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::MemoryUsage));
    // These return time values, so they are functions
    addSystemSubroutine(std::make_shared<SukimaSimSystemFunc>(KnownSystemName::SimTimeNs));
    addSystemSubroutine(std::make_shared<SukimaSimSystemFunc>(KnownSystemName::SimTimePs));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::NClkInfo));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::XmDumpvars));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::NcQuery));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::NcBreak));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::XmTraceOn));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::XmTraceOff));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::XmAssertOn));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::XmAssertOff));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::XmDebug));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::XmLog));
    
    // Phase 1.2: Coverage Extension Tasks (SukimaSim vendor extensions)
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::NcSetSeed));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::ToggleStart));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::ToggleStop));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::ToggleReport));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::FsmStart));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::FsmStop));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::FsmReport));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::GlitchCount));
    
    // Phase 1.3: Performance Monitoring Tasks (SukimaSim vendor extensions)
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::CpuUsage));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::ProfileStart));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::ProfileStop));
    
    // Phase 1.4: File I/O Extension Tasks (SukimaSim vendor extensions)
    // $fopen_compress returns a file handle (integer), so it's a function
    addSystemSubroutine(std::make_shared<SukimaSimSystemFunc>(KnownSystemName::FopenCompress));
    // These are tasks that don't return values
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::FwriteBinary));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::FreadBinary));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::FflushAll));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::Fsync));
    
    // Phase 1.5: Other Extensions (SukimaSim vendor extensions)
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimVersionInfo));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::NcGetHostname));
    // $socket_open returns a socket handle (integer), so it's a function
    addSystemSubroutine(std::make_shared<SukimaSimSystemFunc>(KnownSystemName::SocketOpen));

    // Phase 2+: Advanced SukimaSim Extensions (24 tasks)
    // Debug and Control
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimDebugInfo));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimDebugScope));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimDebugSignal));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimFinishTime));
    // Coverage Extensions
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimCoverageEnable));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimCoverageDisable));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimCoverageSaveIncremental));
    // Performance Monitoring
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimPerfStart));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimPerfStop));
    // File I/O
    addSystemSubroutine(std::make_shared<SukimaSimSystemFunc>(KnownSystemName::SukimaSimFopenExclusive));
    // DPI Extensions
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimDpiExport));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimDpiImport));
    // VPI Extensions
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimVpiRegister));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimVpiIterate));
    // PLI Extensions
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimPliTfCall));
    // Waveform Extensions
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimWaveExtended));
    // Profiling Extensions
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimProfileHier));
    // Advanced Feature Extensions
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimDistSim));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimMemProfile));
    addSystemSubroutine(std::make_shared<SukimaSimSystemFunc>(KnownSystemName::SukimaSimDebugRealtime));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimInteractiveDbg));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimAssertAdvanced));
    // External Tool Integration
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimTlmBridge));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SukimaSimFormalLink));

    // Phase 1 sub-tasks (previously missing registrations)
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::ScopeSet));
    // $scope_get returns scope name (used as function in test code)
    addSystemSubroutine(std::make_shared<SukimaSimSystemFunc>(KnownSystemName::ScopeGet));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::CadenceProbe));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::NcDebug));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::BinaryDump));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::BinaryLoad));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::CompressedDump));
    // $network_socket returns socket handle (integer)
    addSystemSubroutine(std::make_shared<SukimaSimSystemFunc>(KnownSystemName::NetworkSocket));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::MemorySnapshot));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SimulationSpeed));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::CpuUtilization));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::CoverageReportTxt));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::CoverageReportXml));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::CoverageReportHtml));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::ToggleCoverageControl));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::LicensingStatus));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::CommandLineArgs));

    // IEEE 1800-2023 Section 31: Timing Check Tasks
    // These can be used outside of specify blocks
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::Setup));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::Hold));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::SetupHold));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::Recovery));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::Removal));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::RecRem));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::Skew));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::TimeSkew));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::FullSkew));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::Period));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::Width));
    addSystemSubroutine(std::make_shared<SukimaSimSystemTask>(KnownSystemName::NoChange));

    REGISTER(DisplayTask, KnownSystemName::Write, LiteralBase::Decimal);
    REGISTER(DisplayTask, KnownSystemName::WriteB, LiteralBase::Binary);
    REGISTER(DisplayTask, KnownSystemName::WriteO, LiteralBase::Octal);
    REGISTER(DisplayTask, KnownSystemName::WriteH, LiteralBase::Hex);
    REGISTER(MonitorTask, KnownSystemName::Strobe, LiteralBase::Decimal);
    REGISTER(MonitorTask, KnownSystemName::StrobeB, LiteralBase::Binary);
    REGISTER(MonitorTask, KnownSystemName::StrobeO, LiteralBase::Octal);
    REGISTER(MonitorTask, KnownSystemName::StrobeH, LiteralBase::Hex);
    REGISTER(MonitorTask, KnownSystemName::Monitor, LiteralBase::Decimal);
    REGISTER(MonitorTask, KnownSystemName::MonitorB, LiteralBase::Binary);
    REGISTER(MonitorTask, KnownSystemName::MonitorO, LiteralBase::Octal);
    REGISTER(MonitorTask, KnownSystemName::MonitorH, LiteralBase::Hex);

#undef REGISTER
#define REGISTER(type, name) addSystemSubroutine(std::make_shared<type>(name))
    REGISTER(FileDisplayTask, KnownSystemName::FDisplay);
    REGISTER(FileDisplayTask, KnownSystemName::FDisplayB);
    REGISTER(FileDisplayTask, KnownSystemName::FDisplayO);
    REGISTER(FileDisplayTask, KnownSystemName::FDisplayH);
    REGISTER(FileDisplayTask, KnownSystemName::FWrite);
    REGISTER(FileDisplayTask, KnownSystemName::FWriteB);
    REGISTER(FileDisplayTask, KnownSystemName::FWriteO);
    REGISTER(FileDisplayTask, KnownSystemName::FWriteH);
    REGISTER(FileMonitorTask, KnownSystemName::FStrobe);
    REGISTER(FileMonitorTask, KnownSystemName::FStrobeB);
    REGISTER(FileMonitorTask, KnownSystemName::FStrobeO);
    REGISTER(FileMonitorTask, KnownSystemName::FStrobeH);
    REGISTER(FileMonitorTask, KnownSystemName::FMonitor);
    REGISTER(FileMonitorTask, KnownSystemName::FMonitorB);
    REGISTER(FileMonitorTask, KnownSystemName::FMonitorO);
    REGISTER(FileMonitorTask, KnownSystemName::FMonitorH);

    REGISTER(StringOutputTask, KnownSystemName::SWrite);
    REGISTER(StringOutputTask, KnownSystemName::SWriteB);
    REGISTER(StringOutputTask, KnownSystemName::SWriteO);
    REGISTER(StringOutputTask, KnownSystemName::SWriteH);

    REGISTER(StringFormatTask, KnownSystemName::SFormat);

    REGISTER(PrintTimeScaleTask, KnownSystemName::PrintTimeScale);

    REGISTER(DumpVarsTask, KnownSystemName::DumpVars);
    REGISTER(DumpPortsTask, KnownSystemName::DumpPorts);
    REGISTER(ShowVarsTask, KnownSystemName::ShowVars);

    REGISTER(CastTask, KnownSystemName::Cast);

#undef REGISTER
#define REGISTER(type, name, kind) addSystemSubroutine(std::make_shared<type>(name, kind))
    REGISTER(SeverityTask, KnownSystemName::Info, ElabSystemTaskKind::Info);
    REGISTER(SeverityTask, KnownSystemName::Warning, ElabSystemTaskKind::Warning);
    REGISTER(SeverityTask, KnownSystemName::Error, ElabSystemTaskKind::Error);

#undef REGISTER
#define REGISTER(type, name, isFinish) addSystemSubroutine(std::make_shared<type>(name, isFinish))
    REGISTER(FinishControlTask, KnownSystemName::Finish, true);
    REGISTER(FinishControlTask, KnownSystemName::Stop, false);

#undef REGISTER

    addSystemSubroutine(std::make_shared<ReadWriteMemTask>(KnownSystemName::ReadMemB, true));
    addSystemSubroutine(std::make_shared<ReadWriteMemTask>(KnownSystemName::ReadMemH, true));
    addSystemSubroutine(std::make_shared<ReadWriteMemTask>(KnownSystemName::WriteMemB, false));
    addSystemSubroutine(std::make_shared<ReadWriteMemTask>(KnownSystemName::WriteMemH, false));
    addSystemSubroutine(std::make_shared<SReadMemTask>(KnownSystemName::SReadMemB));
    addSystemSubroutine(std::make_shared<SReadMemTask>(KnownSystemName::SReadMemH));
    addSystemSubroutine(std::make_shared<SimpleSystemTask>(KnownSystemName::System, intType, 0,
                                                           std::vector<const Type*>{&stringType}));
    addSystemSubroutine(std::make_shared<SdfAnnotateTask>());
    addSystemSubroutine(std::make_shared<StaticAssertTask>());
    addSystemSubroutine(std::make_shared<FatalTask>());
    addSystemSubroutine(std::make_shared<ScopeTask>(KnownSystemName::List, true));
    addSystemSubroutine(std::make_shared<ScopeTask>(KnownSystemName::Scope, false));

#define TASK(name, required, ...)                                                    \
    addSystemSubroutine(std::make_shared<SimpleSystemTask>(name, voidType, required, \
                                                           std::vector<const Type*>{__VA_ARGS__}))

    TASK(KnownSystemName::Exit, 0, );

    TASK(KnownSystemName::TimeFormat, 0, &intType, &intType, &stringType, &intType);

    TASK(KnownSystemName::MonitorOn, 0, );
    TASK(KnownSystemName::MonitorOff, 0, );
    // IEEE 1800-2023 §21.3.3: $fmonitoron and $fmonitoroff take optional mcd/fd argument
    TASK(KnownSystemName::FMonitorOn, 0, &intType);
    TASK(KnownSystemName::FMonitorOff, 0, &intType);

    TASK(KnownSystemName::DumpFile, 0, &stringType);
    TASK(KnownSystemName::DumpOn, 0, );
    TASK(KnownSystemName::DumpOff, 0, );
    TASK(KnownSystemName::DumpAll, 0, );
    TASK(KnownSystemName::DumpLimit, 1, &intType);
    TASK(KnownSystemName::DumpFlush, 0, );
    TASK(KnownSystemName::DumpPortsOn, 0, &stringType);
    TASK(KnownSystemName::DumpPortsOff, 0, &stringType);
    TASK(KnownSystemName::DumpPortsAll, 0, &stringType);
    TASK(KnownSystemName::DumpPortsLimit, 1, &intType, &stringType);
    TASK(KnownSystemName::DumpPortsFlush, 0, &stringType);

    // Phase 6: Register InputTask with variable arguments
    addSystemSubroutine(std::make_shared<InputTask>(KnownSystemName::Input));
    TASK(KnownSystemName::Key, 0, &stringType);
    TASK(KnownSystemName::NoKey, 0, );
    // TASK(KnownSystemName::Log, 0, &stringType); // REMOVED: VCD $log conflicts with IEEE 1800-2023 math $log
    TASK(KnownSystemName::NoLog, 0, );
    TASK(KnownSystemName::Reset, 0, &intType, &intType, &intType);
    TASK(KnownSystemName::Save, 1, &stringType);
    TASK(KnownSystemName::Restart, 1, &stringType);
    TASK(KnownSystemName::IncSave, 1, &stringType);
    TASK(KnownSystemName::ShowScopes, 0, &intType);
    TASK(KnownSystemName::UpScope, 0, );
    TASK(KnownSystemName::DumpScopes, 0, &stringType);

#undef TASK

#define ASSERTCTRL(name) addSystemSubroutine(std::make_shared<AssertControlTask>(name))
    ASSERTCTRL(KnownSystemName::AssertControl);
    ASSERTCTRL(KnownSystemName::AssertOn);
    ASSERTCTRL(KnownSystemName::AssertOff);
    ASSERTCTRL(KnownSystemName::AssertKill);
    ASSERTCTRL(KnownSystemName::AssertPassOn);
    ASSERTCTRL(KnownSystemName::AssertPassOff);
    ASSERTCTRL(KnownSystemName::AssertFailOn);
    ASSERTCTRL(KnownSystemName::AssertFailOff);
    ASSERTCTRL(KnownSystemName::AssertNonvacuousOn);
    ASSERTCTRL(KnownSystemName::AssertNonvacuousOff);
    ASSERTCTRL(KnownSystemName::AssertVacuousOff);

#undef ASSERTCTRL

    // IEEE 1800-2023 Section 22.5: Stochastic queue functions (function-call form)
    addSystemSubroutine(std::make_shared<QueueFunction>(KnownSystemName::QInitialize, 3, 4));
    addSystemSubroutine(std::make_shared<QueueFunction>(KnownSystemName::QAdd, 3, 4));
    addSystemSubroutine(std::make_shared<QueueFunction>(KnownSystemName::QRemove, 3, 4));
    addSystemSubroutine(std::make_shared<QueueFunction>(KnownSystemName::QExam, 3, 4));
    addSystemSubroutine(std::make_shared<QueueFunction>(KnownSystemName::QFull, 1, 2));

    // There are 16 PLA tasks, with sequential enum values starting with $async$and$array
    for (int i = 0; i < 16; i++) {
        addSystemSubroutine(
            std::make_shared<PlaTask>((KnownSystemName)((int)KnownSystemName::AsyncAndArray + i)));
    }

    // Sukimasim improvements: IEEE 1800-2023 assertion coverage control tasks
    // These fix the "Invalid initial block" issues with FCov tests
    addSystemSubroutine(std::make_shared<SimpleSystemTask>(KnownSystemName::AssertCoveredOn,
                                                          voidType));
    addSystemSubroutine(std::make_shared<SimpleSystemTask>(KnownSystemName::AssertCoveredOff,
                                                          voidType));

    // Add coverage query functions (returning appropriate types)
    addSystemSubroutine(std::make_shared<SimpleSystemTask>(KnownSystemName::AssertionCoverage,
                                                          intType));
    addSystemSubroutine(std::make_shared<SimpleSystemTask>(KnownSystemName::GetAssertionCoverage,
                                                          realType));
    addSystemSubroutine(std::make_shared<SimpleSystemTask>(KnownSystemName::GetToggleCoverage,
                                                          realType));
    addSystemSubroutine(std::make_shared<SimpleSystemTask>(KnownSystemName::SystemCoverage,
                                                          intType));
}

} // namespace slang::ast::builtins

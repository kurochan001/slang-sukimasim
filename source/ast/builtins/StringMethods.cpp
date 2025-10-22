//------------------------------------------------------------------------------
// StringMethods.cpp
// Built-in methods on strings
//
// SPDX-FileCopyrightText: Michael Popoloski
// SPDX-License-Identifier: MIT
//------------------------------------------------------------------------------
#include "Builtins.h"

#include "slang/ast/Compilation.h"
#include "slang/ast/SystemSubroutine.h"
#include "slang/util/String.h"

namespace slang::ast::builtins {

namespace {

// Ensures that the given constant value is represented as a string. For string
// literals evaluated as integers, this will perform the conversion described in
// [6.16]. Returns false if the value cannot be treated as a string.
static bool ensureStringValue(ConstantValue& value) {
    if (!value)
        return false;

    if (!value.isString())
        value = value.convertToStr();

    return value.isString();
}

} // namespace

class StringLenMethod : public SimpleSystemSubroutine {
public:
    explicit StringLenMethod(const Builtins& builtins) :
        SimpleSystemSubroutine(KnownSystemName::Len, SubroutineKind::Function, 0, {},
                               builtins.intType, true) {}

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        ConstantValue val = args[0]->eval(context);
        if (!ensureStringValue(val))
            return nullptr;

        return SVInt(32, val.str().length(), true);
    }
};

class StringPutcMethod : public SimpleSystemSubroutine {
public:
    explicit StringPutcMethod(const Builtins& builtins) :
        SimpleSystemSubroutine(KnownSystemName::Putc, SubroutineKind::Function, 2,
                               {&builtins.intType, &builtins.byteType}, builtins.voidType, true,
                               /* isFirstArgLValue */ true) {}

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        // Phase 2.1: Disable constant evaluation - let SukimaSim handle at runtime
        // String values are runtime entities and should not be constant-evaluated
        return nullptr;
    }
};

class StringGetcMethod : public SimpleSystemSubroutine {
public:
    explicit StringGetcMethod(const Builtins& builtins) :
        SimpleSystemSubroutine(KnownSystemName::Getc, SubroutineKind::Function, 1,
                               {&builtins.intType}, builtins.byteType, true) {}

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        // Phase 2.1: Disable constant evaluation - let SukimaSim handle at runtime
        // String values are runtime entities and should not be constant-evaluated
        return nullptr;
    }
};

class StringUpperLowerMethod : public SimpleSystemSubroutine {
public:
    StringUpperLowerMethod(const Builtins& builtins, KnownSystemName knownNameId, bool upper) :
        SimpleSystemSubroutine(knownNameId, SubroutineKind::Function, 0, {}, builtins.stringType,
                               true),
        upper(upper) {}

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        ConstantValue val = args[0]->eval(context);
        if (!ensureStringValue(val))
            return nullptr;

        std::string& str = val.str();
        if (upper)
            strToUpper(str);
        else
            strToLower(str);
        return val;
    }

private:
    bool upper;
};

class StringCompareMethod : public SimpleSystemSubroutine {
public:
    StringCompareMethod(const Builtins& builtins, KnownSystemName knownNameId, bool ignoreCase) :
        SimpleSystemSubroutine(knownNameId, SubroutineKind::Function, 1, {&builtins.stringType},
                               builtins.intType, true),
        ignoreCase(ignoreCase) {}

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        ConstantValue lhsCv = args[0]->eval(context);
        ConstantValue rhsCv = args[1]->eval(context);
        if (!ensureStringValue(lhsCv) || !ensureStringValue(rhsCv))
            return nullptr;

        std::string& lhs = lhsCv.str();
        std::string& rhs = rhsCv.str();

        int result;
        if (ignoreCase) {
            // No convenient way to do this with standard lib functions :(
            const char* p1 = lhs.c_str();
            const char* p2 = rhs.c_str();
            while ((result = charToLower(*p1) - charToLower(*p2++)) == 0) {
                if (*p1++ == '\0')
                    break;
            }
        }
        else {
            result = lhs.compare(rhs);
            if (result < 0)
                result = -1;
            else if (result > 0)
                result = 1;
        }

        return SVInt(32, uint64_t(result), true);
    }

private:
    bool ignoreCase;
};

class StringSubstrMethod : public SimpleSystemSubroutine {
public:
    explicit StringSubstrMethod(const Builtins& builtins) :
        SimpleSystemSubroutine(KnownSystemName::Substr, SubroutineKind::Function, 2,
                               {&builtins.intType, &builtins.intType}, builtins.stringType, true) {}

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        ConstantValue strCv = args[0]->eval(context);
        auto leftCv = args[1]->eval(context);
        auto rightCv = args[2]->eval(context);
        if (!ensureStringValue(strCv) || !leftCv || !rightCv)
            return nullptr;

        const std::string& str = strCv.str();
        int32_t left = leftCv.integer().as<int32_t>().value();
        int32_t right = rightCv.integer().as<int32_t>().value();
        if (left < 0 || right < left || size_t(right) >= str.length())
            return ""s;

        int32_t count = right - left + 1;
        return str.substr(size_t(left), size_t(count));
    }
};

class StringAtoIMethod : public SimpleSystemSubroutine {
public:
    StringAtoIMethod(const Builtins& builtins, KnownSystemName knownNameId, int /* base */) :
        SimpleSystemSubroutine(knownNameId, SubroutineKind::Function, 0, {}, builtins.integerType,
                               true) {}

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        // Phase 2.1: Disable constant evaluation - let SukimaSim handle at runtime
        // String values are runtime entities and should not be constant-evaluated
        return nullptr;
    }
};

class StringAtoRealMethod : public SimpleSystemSubroutine {
public:
    explicit StringAtoRealMethod(const Builtins& builtins) :
        SimpleSystemSubroutine(KnownSystemName::AToReal, SubroutineKind::Function, 0, {},
                               builtins.realType, true) {}

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        // Phase 2.1: Disable constant evaluation - let SukimaSim handle at runtime
        // String values are runtime entities and should not be constant-evaluated
        return nullptr;
    }
};

class StringItoAMethod : public SimpleSystemSubroutine {
public:
    StringItoAMethod(const Builtins& builtins, KnownSystemName knownNameId, LiteralBase /* base */) :
        SimpleSystemSubroutine(knownNameId, SubroutineKind::Function, 1, {&builtins.integerType},
                               builtins.voidType, true, /* isFirstArgLValue */ true) {}

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        // Phase 2.1: Disable constant evaluation - let SukimaSim handle at runtime
        // String values are runtime entities and should not be constant-evaluated
        return nullptr;
    }
};

class StringRealtoAMethod : public SimpleSystemSubroutine {
public:
    explicit StringRealtoAMethod(const Builtins& builtins) :
        SimpleSystemSubroutine(KnownSystemName::RealToA, SubroutineKind::Function, 1,
                               {&builtins.realType}, builtins.voidType, true,
                               /* isFirstArgLValue */ true) {}

    ConstantValue eval(EvalContext& context, const Args& args, SourceRange,
                       const CallExpression::SystemCallInfo&) const final {
        // Phase 2.1: Disable constant evaluation - let SukimaSim handle at runtime
        // String values are runtime entities and should not be constant-evaluated
        return nullptr;
    }
};

void Builtins::registerStringMethods() {
    using parsing::KnownSystemName;

#define REGISTER(kind, name, ...) addSystemMethod(kind, std::make_unique<name##Method>(__VA_ARGS__))
    REGISTER(SymbolKind::StringType, StringLen, *this);
    REGISTER(SymbolKind::StringType, StringPutc, *this);
    REGISTER(SymbolKind::StringType, StringGetc, *this);
    REGISTER(SymbolKind::StringType, StringUpperLower, *this, KnownSystemName::ToUpper, true);
    REGISTER(SymbolKind::StringType, StringUpperLower, *this, KnownSystemName::ToLower, false);
    REGISTER(SymbolKind::StringType, StringCompare, *this, KnownSystemName::Compare, false);
    REGISTER(SymbolKind::StringType, StringCompare, *this, KnownSystemName::ICompare, true);
    REGISTER(SymbolKind::StringType, StringSubstr, *this);
    REGISTER(SymbolKind::StringType, StringAtoI, *this, KnownSystemName::AToI, 10);
    REGISTER(SymbolKind::StringType, StringAtoI, *this, KnownSystemName::AToHex, 16);
    REGISTER(SymbolKind::StringType, StringAtoI, *this, KnownSystemName::AToOct, 8);
    REGISTER(SymbolKind::StringType, StringAtoI, *this, KnownSystemName::AToBin, 2);
    REGISTER(SymbolKind::StringType, StringAtoReal, *this);
    REGISTER(SymbolKind::StringType, StringItoA, *this, KnownSystemName::IToA,
             LiteralBase::Decimal);
    REGISTER(SymbolKind::StringType, StringItoA, *this, KnownSystemName::HexToA, LiteralBase::Hex);
    REGISTER(SymbolKind::StringType, StringItoA, *this, KnownSystemName::OctToA,
             LiteralBase::Octal);
    REGISTER(SymbolKind::StringType, StringItoA, *this, KnownSystemName::BinToA,
             LiteralBase::Binary);
    REGISTER(SymbolKind::StringType, StringRealtoA, *this);

#undef REGISTER
}

} // namespace slang::ast::builtins

// SPDX-FileCopyrightText: Michael Popoloski
// SPDX-License-Identifier: MIT

#include "Test.h"

#include "slang/ast/Constraints.h"
#include "slang/ast/expressions/MiscExpressions.h"
#include "slang/ast/symbols/ClassSymbols.h"

TEST_CASE("Constraint metadata surfaces solve/soft/dist information") {
    auto tree = SyntaxTree::fromText(R"(
class Item;
    rand int a;
    rand int b;

    constraint ctrl {
        soft a inside {[0:5]};
        solve a before b;
        b dist { 0 := 1, [1:3] :/ 2, default :/ 4 };
    }
endclass
)");

    Compilation compilation;
    compilation.addSyntaxTree(tree);
    NO_COMPILATION_ERRORS;

    auto& item = compilation.getRoot().compilationUnits[0]->lookupName<ClassType>("Item");
    auto& ctrl = item.find<ConstraintBlockSymbol>("ctrl");

    auto& list = ctrl.getConstraints().as<ConstraintList>();
    REQUIRE(list.list.size() == 3);

    const auto* softExpr = list.list[0]->as_if<ExpressionConstraint>();
    REQUIRE(softExpr);
    CHECK(softExpr->isSoft);
    auto softSym = softExpr->expr.getSymbolReference();
    REQUIRE(softSym);
    CHECK(softSym == &item.find<ClassPropertySymbol>("a"));

    const auto* solveBefore = list.list[1]->as_if<SolveBeforeConstraint>();
    REQUIRE(solveBefore);
    auto solveVars = solveBefore->getSolveVariables();
    auto afterVars = solveBefore->getAfterVariables();
    REQUIRE(solveVars.size() == 1);
    REQUIRE(afterVars.size() == 1);
    CHECK(solveVars[0] == &item.find<ClassPropertySymbol>("a"));
    CHECK(afterVars[0] == &item.find<ClassPropertySymbol>("b"));

    const auto* distExprConstraint = list.list[2]->as_if<ExpressionConstraint>();
    REQUIRE(distExprConstraint);
    CHECK_FALSE(distExprConstraint->isSoft);
    REQUIRE(distExprConstraint->expr.kind == ExpressionKind::Dist);

    auto& distExpr = distExprConstraint->expr.as<DistExpression>();
    auto items = distExpr.items();
    REQUIRE(items.size() == 2);
    REQUIRE(items[0].weight);
    CHECK(items[0].weight->kind == DistExpression::DistWeight::PerValue);
    REQUIRE(items[1].weight);
    CHECK(items[1].weight->kind == DistExpression::DistWeight::PerRange);

    auto* defaultWeight = distExpr.defaultWeight();
    REQUIRE(defaultWeight);
    CHECK(defaultWeight->kind == DistExpression::DistWeight::PerValue);
}

TEST_CASE("Soft constraint requires random variable") {
    auto tree = SyntaxTree::fromText(R"(
class Item;
    int value;
    constraint ctrl { soft value == 0; }
endclass
)");

    Compilation compilation;
    compilation.addSyntaxTree(tree);
    auto& diags = compilation.getAllDiagnostics();
    REQUIRE(diags.size() == 1);
    CHECK(diags[0].code == diag::SoftConstraintNoRand);
}

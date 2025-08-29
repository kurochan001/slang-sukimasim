// Phase 182: Patch for correct localparam evaluation in generate blocks
// This file contains a modified createBlock lambda that ensures localparams
// are evaluated with the correct genvar context

#include "slang/ast/symbols/BlockSymbols.h"
#include "slang/ast/symbols/ParameterSymbols.h"
#include "slang/ast/ASTContext.h"
#include "slang/ast/Compilation.h"

// This is a conceptual patch - the actual implementation would need to be
// integrated into BlockSymbols.cpp around line 759-780

/*
Original createBlock lambda (line 759-780):
    auto createBlock = [&, blockLoc = loc](ConstantValue value, bool isUninstantiated) {
        // Spec: each generate block gets their own scope, with an implicit
        // localparam of the same name as the genvar.
        auto block = comp.emplace<GenerateBlockSymbol>(comp, "", blockLoc, (uint32_t)entries.size(),
                                                       isUninstantiated);
        auto implicitParam = comp.emplace<ParameterSymbol>(genvar.valueText(), genvar.location(),
                                                           true /* isLocal */, false /* isPort */);
        implicitParam->setSyntax(*genvarSyntax);
        comp.noteReference(*implicitParam);

        block->addMember(*implicitParam);
        block->setSyntax(*syntax.block);

        addBlockMembers(*block, *syntax.block);

        implicitParam->setType(comp.getIntegerType());
        implicitParam->setValue(comp, std::move(value), /* needsCoercion */ false);
        implicitParam->setIsFromGenvar(true);

        block->arrayIndex = &implicitParam->getValue().integer();
        entries.push_back(block);
    };

Phase 182 Modified version:
*/
auto createBlockFixed = [&, blockLoc = loc](ConstantValue value, bool isUninstantiated) {
    // Spec: each generate block gets their own scope, with an implicit
    // localparam of the same name as the genvar.
    auto block = comp.emplace<GenerateBlockSymbol>(comp, "", blockLoc, (uint32_t)entries.size(),
                                                   isUninstantiated);
    auto implicitParam = comp.emplace<ParameterSymbol>(genvar.valueText(), genvar.location(),
                                                       true /* isLocal */, false /* isPort */);
    implicitParam->setSyntax(*genvarSyntax);
    comp.noteReference(*implicitParam);

    // Phase 182: Set the implicit parameter value BEFORE adding block members
    implicitParam->setType(comp.getIntegerType());
    implicitParam->setValue(comp, value.clone(comp.getConstantAllocator()), /* needsCoercion */ false);
    implicitParam->setIsFromGenvar(true);
    
    // Add the implicit parameter to the block first
    block->addMember(*implicitParam);
    block->setSyntax(*syntax.block);
    
    // Phase 182: Create a temporary context that includes the implicit parameter
    // This ensures that when localparams are evaluated, they can see the genvar value
    ASTContext tempContext(*block, LookupLocation::max);
    
    // Add block members - this will evaluate localparams with the correct context
    addBlockMembers(*block, *syntax.block);
    
    // Phase 182: Force re-evaluation of all localparams in the block
    // This ensures they get the correct genvar values
    for (auto& member : block->members()) {
        if (member.kind == SymbolKind::Parameter) {
            auto& param = member.as<ParameterSymbol>();
            if (!param.isFromGenvar()) {
                // Force re-evaluation with the correct context
                // This would require access to the parameter's internals
                // which may need additional API changes
                
                // Conceptual code:
                // param.clearValue();
                // param.getValue(); // This will trigger re-evaluation
            }
        }
    }

    block->arrayIndex = &implicitParam->getValue().integer();
    entries.push_back(block);
};

/*
Alternative approach: Modify ParameterSymbol::getValue() to check if we're in a 
generate block and look up the genvar value from the parent scope.

In ParameterSymbols.cpp around line 145-148:
    auto init = getInitializer();
    if (init) {
        // Phase 182: Check if we're in a generate block
        // and if the initializer references a genvar
        if (scope->kind == SymbolKind::GenerateBlock) {
            // Look for implicit parameters that are from genvar
            for (auto& member : scope->members()) {
                if (member.kind == SymbolKind::Parameter) {
                    auto& param = member.as<ParameterSymbol>();
                    if (param.isFromGenvar()) {
                        // Make this parameter visible in the evaluation context
                        // This would require modifying the ASTContext
                    }
                }
            }
        }
        
        value = scope->getCompilation().allocConstant(
            ctx.eval(*init, EvalFlags::AllowUnboundedPlaceholder));
    }
*/
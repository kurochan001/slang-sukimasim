# slang-sukimasim Code Review (2026-03-02)

## Overview

Full code review of the slang-sukimasim fork (branch: `sukimasim-improvements`).
- Divergence from upstream: 296 commits, 153 source files changed (+33,137/-5,051 lines)
- Review scope: All sukimasim-specific modifications to the slang parser/AST

## Summary

| Category | Count | Action |
|----------|-------|--------|
| Actual bug (fixed) | 1 | B3 |
| Dead code/cleanup (fixed) | 4 | CL1-CL4 |
| getenv cache optimization (fixed) | 4 files | Performance |
| sukimasim runtime extension (no action) | 2 | B1, B2 — matches runtime |
| Intentional design (no action) | 7 | Runtime delegation |
| False positives (no action) | 5 | Correct as-is |
| **Total findings** | **24** | |

## Bug Fixed

### B3 (Medium): Drive strength error recovery default — IEEE §10.3.3

**File**: `source/parsing/Parser.cpp` (parseDriveStrength)

**Issue**: Error recovery inserted Pull strength tokens. IEEE §10.3.3 default is Strong.

**Fix**: `Pull1Keyword`→`Strong1Keyword`, `Pull0Keyword`→`Strong0Keyword`

## sukimasim Runtime Extensions (No Action — Matches Runtime)

### B1: $sreadmemb/$sreadmemh argument order

**File**: `source/ast/builtins/SystemTasks.cpp` (SReadMemTask)

**Finding**: Argument order is (string_data, memory, [start], [finish]) which differs from
IEEE §21.4: `$sreadmemb(mem_variable, start_addr, finish_addr, string1, ...)`.

**Decision**: No change. The sukimasim runtime (`system_task_handlers.cpp:8163`) expects
arg0=string, arg1=memory. Parser must match runtime. This is a documented sukimasim extension.

### B2: $getpattern 4-argument signature

**File**: `source/ast/builtins/NonConstFuncs.cpp` (GetPatternFunc)

**Finding**: Accepts 4 arguments (memory, start, end, output_string) while IEEE §20.17.3
specifies 1 argument.

**Decision**: No change. The sukimasim runtime (`testbench_runtime.cpp:8477`) expects
4 arguments. Parser must match runtime. This is a documented sukimasim extension.

## Dead Code / Cleanup

### CL1 (Low): if(false) dead code block

**File**: `source/ast/expressions/MiscExpressions.cpp` (L413)

Removed `if (false) { ... }` block (covergroup expression check disabled by flag removal).

### CL2 (Low): isValidRange always returns true

**File**: `source/ast/builtins/SystemTasks.cpp` (PlaTask)

`isValidRange()` always returned true per IEEE 1800-2023 Annex G (both ascending/descending
ranges allowed). Removed `isValidRange()`, `badRange()`, and all call sites.
Updated `tests/unittests/ast/SystemFuncTests.cpp` accordingly (9→5 diagnostics).

### CL3 (Low): Verbose error message in array size check

**File**: `source/ast/expressions/AssignmentExpressions.cpp` (L558)

Changed `<< "Array size too large: " + std::to_string(count)` to `<< count` to match
the `diag::InvalidArraySize` format string `"{} is not a valid size for an array"`.

### CL4 (Low): void port type diagnostic suppression

**File**: `source/ast/symbols/PortSymbols.cpp` (L1348)

Removed `!type->isVoid()` from diagnostic suppression condition. Void is not a valid
port type and should produce a diagnostic.

## getenv Cache Optimization

Cached `std::getenv()` calls to `static const bool` to avoid repeated syscalls:

| File | Variable | Environment Variable |
|------|----------|---------------------|
| `source/ast/Compilation.cpp` (2 scopes) | `dbg_bind` | `SUKIMASIM_DEBUG_BIND` |
| `source/ast/expressions/CallExpression.cpp` | `dbg_ifmethod` | `SUKIMASIM_DEBUG_INTERFACE_METHOD` |
| `source/ast/expressions/SelectExpressions.cpp` | `dbg_ifmethod` | `SUKIMASIM_DEBUG_INTERFACE_METHOD` |
| `source/ast/types/DeclaredType.cpp` | `defer_proc_init` | `SUKIMASIM_DEFER_PROCEDURAL_INIT` |

## Intentional Design Choices (No Action Required)

| ID | Finding | Reason |
|----|---------|--------|
| H-02 | Enum method eval disabled | sukimasim runtime handles all enum methods |
| H-03 | String method eval disabled | sukimasim runtime handles all string methods |
| H-06 | Preprocessor simplification | sukimasim doesn't use keyword mapping/macro frames |
| H-07 | Assertion control validation relaxed | Runtime validates assertion control |
| M-01 | $dumpports arg validation relaxed | Runtime validates arguments |
| M-08 | Coverage function return type changed | Actually an IEEE compliance fix (upstream was wrong) |
| L-06 | $log commented out | Name collision with math $log; handled by sukimasim runtime |

## False Positives (No Issue)

| ID | Finding | Reason |
|----|---------|--------|
| C-01 | EvaluatingPlaceholder removed | Actually present in VariableSymbols.cpp; viaArgExpansion is legitimate |
| H-01 | Unconditional cerr output | All guarded by getenv() checks |
| H-04 | Unary operator precedence | INT_MAX prevents consuming binary operators; `!a+b`→`(!a)+b` correct |
| M-05 | ArrayMethods_phase176.cpp | File does not exist |
| M-04 | isValidForPort relaxation | Required for class/virtual interface port support |

## Codex Follow-up Review (2026-03-02)

3 findings from codex review — all resolved:

1. **High (Closed)**: Annex D unit test / fork implementation mismatch
   - `$getpattern` call updated to 4-arg form, `$sreadmemb`/`$sreadmemh` arg order fixed
   - Expected diagnostics updated (9→6)
2. **Medium (Closed)**: `PlaRangeInAscendingOrder` diagnostic definition removed from `diagnostics.txt`
3. **Low (Closed)**: getenv static cache constraint documented in CLAUDE.md

## Verification

- slang-sukimasim: Build clean (no errors/warnings)
- slang-sukimasim unit tests: PLA, Annex D, I/O, drive strength — all pass
- sukimasim: Build clean
- CTest: 1,263/1,263 (100%)
- Extended tests: 16,705/16,705 (100%)

---
Reviewed by: Claude Code + Codex (2026-03-02)

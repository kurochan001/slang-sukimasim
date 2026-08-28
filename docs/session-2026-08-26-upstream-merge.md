# upstream/master マージ (2026-08-26)

`upstream/master` (a5ab85255) を `sukimasim-improvements` (1fbb62a6e) に取り込んだ記録。
前回同期点 `ad719280d` (2025-10-23) から **565 コミット / 10ヶ月分**。

マージコミット: `1f529a85c` (2親: 1fbb62a6e + a5ab85255)

## 事前準備: shallow clone の解除

このリポジトリは shallow clone だったため `git merge-base` が空を返し、upstream との
共通祖先が存在しないように見えていた。`git fetch --unshallow upstream` が必須。
解除前は upstream ログの最古が 2026-01-10 で、2025-08〜2026-01 のコミットが不可視だった。

## ビルド時の注意 (既存 build/ を使う場合)

旧 `external/CMakeLists.txt` が設定していた `MI_BUILD_OBJECT:INTERNAL=OFF` が
CMakeCache に残存する。upstream はこの指定を削除し、逆に `mimalloc-obj` を要求するため
cmake が `Could not find mimalloc-obj target` で失敗する。

```bash
cmake -B build -U MI_BUILD_OBJECT
```

## 衝突解決 (38ファイル / 63ハンク)

| 箇所 | 対応 |
|---|---|
| `ASTContext.h` / `Lookup.h` | fork 追加フラグのビット番号が upstream と衝突。ASTFlags 44/45、LookupFlags 15/16 へ再配置。LookupFlags の `SLANG_BITMASK` 上限が実最大より低い潜在バグも修正 |
| `CompatMode` | upstream が `CompatSettings.h` へ移動。fork の Strict モードを移植。9診断のうち8件は upstream の既定分岐が担当済みで、`MissingTimeScale` のみ追加が必要 |
| `Driver.cpp` | upstream の `CompatSettings` を採用し、fork の Phase 151 既定モード緩和を `configureDiagnostics` の後段で再適用 |
| `resolveDefParamsAndBinds` | upstream が `DefParamResolver.cpp` へ移動。`SUKIMASIM_DEBUG_BIND` トレースも移動 |
| パッケージ export | upstream が public な `exportDecls` を private な `ExportData` に置換。fork の再エクスポート解決と parameterized package 生成 (§26.2) を新構造へ移植。`GenericPackageDefSymbol` に friend 宣言追加 |
| `MiscExpressions.cpp` | upstream が `checkLValue` を抽出。fork の `ProceduralForceAssign` 例外を移植 |
| `Parser_members.cpp` | upstream が `parseMember`/`parseMemberImpl` に分割。fork の constexpr スキップを Impl 先頭へ。fork が移動済みの変数宣言ブロックへ upstream の `PackageImportInClass`/`LetDeclaration` 分岐を移植 |
| `diagnostics.txt` | `CannotIndexScalar` の重複定義を回避。`IndexValueInvalid` は fork の warning 版を採用。警告グループは和集合 |
| テスト4ファイル | upstream 版を採用 |

## テキスト衝突なしに壊れていた箇所

git が衝突と判定しなかった領域で、コンパイルエラー/意味破壊が9件。

- `SelectExpressions.cpp` — upstream が引数名を `compilation` → `comp` に変更。fork 追加行が旧名のまま自動マージされた (885-1151行を一括修正)
- `Compilation.h` — `getDPIExports()` が戻り値型違いで二重定義
- `Preprocessor.h` — `splitTokens` が二重宣言 (upstream が public 化)
- `CompatSettings.cpp` — `CompilationDiags.h` の include 欠落
- `Parser.cpp` / `Parser_expressions.cpp` / `Parser_statements.cpp` — `std::span` → 型付き `SyntaxList`/`SeparatedSyntaxList` の API 変更に未追従
- `Parser_statements.cpp` — upstream の `isStmt` 宣言・代入が消失
- `MemberSymbols.cpp` — upstream が削除した `isFromExport` を参照
- `CompilationUnitSymbols.cpp` — 削除された `exportDecls` を参照
- `CoverSymbols.cpp` — `IntegerLiteral::fromConstant` の第1引数が `Compilation&` → `const TypeProvider&`
- `MiscExpressions.h/.cpp` — fork 独自の `MatchesExpression` に upstream 新設の `isEquivalentImpl` が未実装

## テスト結果

| | ベースライン (1fbb62a6e) | マージ後 |
|---|---|---|
| テスト総数 | 1991 | 2493 (upstream が502件追加) |
| 失敗 | 55 | 76 |

**CLAUDE.md の「54件の意図的差異」は 76件 に更新が必要。**

- 解消された既存失敗: 4件 (Complicated lvalue path / Dynamic array eval /
  Multiple ranges split between macro and not / User-defined nettype port connection errors)
- 新規失敗: 25件

### 新規失敗25件の帰属

| 分類 | 件数 |
|---|---|
| upstream が新規追加したテスト | 17 |
| upstream が期待値を更新した既存テスト | 7 |
| 個別に root cause 確定 | 1 |

root cause 確定分 — `Deferred assertion error cases`:
fork が既存で持つ `NamedValueExpression::evalImpl` の改変
(`ASTFlags::TopLevelStatement` / `NonProcedural` のとき評価失敗せずローカルを合成) により
`ASTContext::tryEval` が成功し、upstream 新設の `-Wbits-of-integer-constant` が
`void'($bits(i))` 文脈で発火する。`$display($bits(i))` では発火しない。マージ起因ではない。

残る24件は「upstream 側テストの出自」による分類であり、1件ずつの個別確認は未実施。

### 新規失敗の全リスト

```
Additional explicit port expression checks
Additional implicit port type mismatch checking
Annex D option system tasks and functions
Ansi port initializers
Array reduction method errors
Cover cross with dotted member access
Coverage function in constant context
Covergroup coverage expr forward reference errors
Deferred assertion error cases
Enum method eval with non-const args
Enum method hierarchical reference errors
Implicit net creation with missing identifier in port connection -- GH #1888
More port connection tests
Multi assign through ref ports 2
No latch inferred for always_latch
No range select ordering error for single bit value
Non-ansi port errors
Range order mismatch error suppressed in untaken conditionals
Range select out of bounds during constant eval
String method eval with non-const args
System function args count as outputs
Typed input port with non-net data type -- GH #1853
Unrollable for loop drivers -- strict checking
ValuePath toString - element select dynamic index
foreach loop with function call as array name
```

## 維持したフォーク挙動

LintMode の寛容化、Phase 151 の重要度緩和、Phase 2.1 の文字列/カバレッジ実行時評価、
cover cross の階層参照 (§19.6.1)、インタフェース配列 foreach (§12.7.3)、
インライン assertion 局所変数、`constexpr` 修飾子、expect 文、柔軟な net 型、
IEEE 1800-2023 Strict 互換モード。

## 未検証領域

pyslang (upstream が pybind11 → nanobind 移行) は `SLANG_INCLUDE_PYLIB` が off のため
ビルド・検証していない。

## リグレッションスイート (ctest -R regression)

マージ直後は 3件中3件失敗。内訳と対応:

| テスト | 原因 | 状態 |
|---|---|---|
| `regression_all_file` | upstream が all.sv に `bins sf = arr;` を追加。fork の `bindCovergroupExpr` が定数評価するため `ConstEvalNonConstVariable` | **修正済** (aac0b3420) — 当該箇所のみ `Expression::bind` へ戻した |
| `regression_cst_json_gen` | 上と同一原因 | **修正済** |
| `regression_cst_json_roundtrip` | **マージ前から失敗していた既存バグ**。fork commit `bf0b82e90` が `LexerFacts::getTokenKindText` から `strong0/strong1/weak0/weak1` の case を削除したため、これらのトークンの `rawText()` が空文字を返す。CST JSON が `"kind":"Weak1Keyword","text":""` となり再構築で欠落する | **未修正** |

### regression_cst_json_roundtrip の再現と修正案

```systemverilog
module m;
    wor [1:0] w;
    assign (supply0, weak1) w = 2;   // 再構築時に weak1 が消える
endmodule
```

`source/parsing/LexerFacts.cpp` の `getTokenKindText` に以下4行を復元すれば直る見込み
(upstream・merge-base の双方に存在する。`scripts/tokenkinds.txt` の
`Weak1Keyword` 等の定義は fork にも残っているため、case の復元のみで足りるはず):

```cpp
        case TokenKind::Strong0Keyword: return "strong0";
        case TokenKind::Strong1Keyword: return "strong1";
        case TokenKind::Weak0Keyword: return "weak0";
        case TokenKind::Weak1Keyword: return "weak1";
```

マージ起因ではないため今回は手を入れていない。

## 後日談: 取り残した dead code (2026-08-28)

マージ後に macOS でビルドしたところ `-Werror` で失敗し、`f7580725f` で修正された。

`Parser_members.cpp` の cover cross 衝突 (上表の「fork の階層参照許容」) で fork 側を採用した際、
`nameHasSelects()` の呼び出しが両方とも消え、**static 関数の定義だけが残った**。
自分自身の再帰呼び出ししか参照が無い状態になり、Apple clang 21 の
`-Wunneeded-internal-declaration` が発火した。

```cpp
// 削除された、呼び出し元のない関数
static bool nameHasSelects(const NameSyntax& name) { ... }
```

### 教訓

- **Linux の gcc はこれを警告しない。** マージ検証 (ビルド + unittests 2493件 +
  リグレッション) はすべて Linux/gcc で行っており、全て通ったため見逃した
- 「呼び出し側を削除したが定義が残る」パターンは衝突解決で発生しやすい。
  解決したファイルの static 関数について呼び出し元の有無を確認すること
- 他に同種の取り残しが無いことは、解決した12ファイルを走査して確認済み
- **プラットフォーム依存の警告差は他にも残っている可能性がある** (MSVC 等は未検証)

## 現在の意図的差異 76件（基準リスト）

`803ab9934` 時点。`./build/bin/unittests` は 2493件中76件失敗する。
これが CLAUDE.md の「76件の意図的差異」の実体。**新しい変更を入れたあと、この一覧と
差分が出たら退行**とみなす。再生成は:

```bash
./build/bin/unittests -r xml 2>/dev/null | grep -oP "(?<=<TestCase name=\")[^\"]*"  # 全件
```

```
$sformat invalid %p call
$typename
Accessing program objects from modules is disallowed
Additional explicit port expression checks
Additional implicit port type mismatch checking
Annex D option system tasks and functions
Ansi port initializers
Array query functions
Array reduction method errors
Array select out of bounds - invalid
Array select out of bounds - valid
Binary operator warnings
Class constructor calls
Concurrent assertion expression errors
Conditional matching expression
Constraint metadata surfaces solve/soft/dist information
Cover cross bin set expressions
Cover cross illegal matches expr
Cover cross with dotted member access
Cover points
Coverage function in constant context
Coverage options
Coverage system functions
Covergroup basic errors
Covergroup coverage expr forward reference errors
Covergroup formals are const
Coverpoint bins
Coverpoint non-constant bins expressions
Coverpoints and cover cross name lookup
Cycle delay errors
DPI Imports
DPI signature checking
Deferred assertion error cases
Distribution functions
Enum method eval with non-const args
Enum method hierarchical reference errors
Enum method lookup
Eval enum methods
Eval string methods
Format string - errors
Implicit net creation with missing identifier in port connection -- GH #1888
Invalid continuous assign
Invalid name component lookup handling
Invalid nets
Invalid unpacked dimensions
JSON dump -- covergroup with an option
Methods allowed in constant context
More port connection tests
Multi assign through ref ports 2
Multi assign via mutually referential interfaces
New array expression
No latch inferred for always_latch
No range select ordering error for single bit value
Non-ansi port errors
Non-const name selector
Out-of-bounds range selects in consteval
Range order mismatch error suppressed in untaken conditionals
Range select out of bounds during constant eval
Sampled value functions
Selects with negative bounds
Stochastic tasks
Streaming concat in non-stream context
Streaming operators
String method eval with non-const args
System function args count as outputs
Task / constructor parse errors
Typed input port with non-net data type -- GH #1853
Unrollable for loop drivers -- strict checking
Unused param defaults are still checked for correctness
ValuePath toString - element select dynamic index
Visit all file
bit-stream cast evaluation
foreach loop with function call as array name
v1800-2023: derived covergroups
v1800-2023: real coverpoint errors
v1800-2023: solve-before with reals, array sizes```

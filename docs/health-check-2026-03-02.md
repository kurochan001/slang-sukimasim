# slang-sukimasim ヘルスチェック (2026-03-02)

## 対象

- ブランチ: `sukimasim-improvements`
- HEAD: `be413f626`
- upstream からの差分: 249 コミット、153 ファイル変更 (+33,137/-5,051 行)

## 総合判定

**致命的な問題なし。急ぎの修正不要。**

| 項目 | 状態 |
|------|------|
| ビルド | クリーン (エラー・警告ゼロ) |
| ユニットテスト | 1936/1990 通過 (54件は意図的差異) |
| CTest | 4件失敗 (全て意図的差異に起因) |
| セキュリティ/UB | 該当なし |
| メモリ安全性 | 問題なし |
| スレッド安全性 | 問題なし |

---

## 安全性確認済みの項目

### メモリ管理

- `SVInt` (`source/numeric/SVInt.cpp`): `new uint64_t[]`/`delete[]` が適切にペア化。独自の値セマンティクスでコピー/ムーブ/デストラクタを実装
- `ParserBase::Window` (`source/parsing/ParserBase.h:52-59`): `new Token[]`/`delete[]` が適切にペア化。容量倍増時もコピー後に delete
- `TempBuffer` (`source/numeric/SVIntHelpers.h:76-94`): RAII でヒープ/スタック切り替え

### reinterpret_cast 使用箇所

- `Token::Info::extra()` (`source/parsing/Token.cpp:32-39`): アライメント検証済み (`SLANG_ASSERT(extra % alignof(void*) == 0)`)
- `PointerMap` (`source/ast/symbols/InstanceSymbols.cpp:766-936`): ポインタ同一性をキーに使用。null チェック済み
- センチネル値 (`source/ast/symbols/VariableSymbols.cpp:349`, `source/ast/types/DeclaredType.cpp:32`): `UINTPTR_MAX` を再帰検出用に使用。デリファレンスなし

### const_cast 使用箇所

- `DriverTracker` (`source/analysis/DriverTracker.cpp:183-203`): コメント付き、解析中の変更なし
- その他 `source/ast/` 内: 新規作成オブジェクトへの書き込みで使用。安全

### getenv() キャッシュ

全箇所 `static const bool` で適切にキャッシュ済み:

| ファイル | 環境変数 |
|----------|----------|
| `source/ast/Compilation.cpp:2115, 2372` | `SUKIMASIM_DEBUG_BIND` |
| `source/ast/expressions/CallExpression.cpp:55` | `SUKIMASIM_DEBUG_INTERFACE_METHOD` |
| `source/ast/expressions/SelectExpressions.cpp:909` | `SUKIMASIM_DEBUG_INTERFACE_METHOD` |
| `source/ast/types/DeclaredType.cpp:443` | `SUKIMASIM_DEFER_PROCEDURAL_INIT` |

C++11 magic statics 保証によりスレッドセーフ。

### スレッド安全性

- ミュータブルなファイルスコープ static 変数なし (`source/ast/`, `source/parsing/`)
- `SLANG_USE_THREADS` パス (`source/analysis/AnalysisManager.cpp`): スレッドプールがスレッドインデックスで状態を分離

---

## 改善推奨事項 (緊急性なし)

### 1. ソースツリーの古い生成ファイル (Medium, 修正済み)

**場所**: `source/slang/parsing/KnownSystemName.h`, `source/slang/syntax/KnownSystemName.cpp`

`scripts/systemnames.txt` と同期していない。ビルドは `${CMAKE_CURRENT_BINARY_DIR}` の生成物を使うため実害なし。ただし:
- ソースツリー版は 376 エントリ、ビルド生成版は 396 エントリ
- `KnownSystemName.cpp` に `"#"` キーの重複エントリあり (L654, L655, L796)
- `$srandom`, timing check 関連の新エントリがソースツリー版に欠落

**対応**:
- `scripts/syntax_gen.py` にコメント行フィルタ (`line.startswith("#")`) を追加 — `"#"` 重複エントリの根本原因を修正
- ソースツリーの古い生成ファイル3件を `git rm` で削除 (`source/slang/parsing/KnownSystemName.h`, `slang/parsing/KnownSystemName.h`, `slang/slang/parsing/KnownSystemName.h`)
- 未追跡の `source/slang/syntax/KnownSystemName.cpp` も削除
- ビルド・テスト回帰なし

### 2. CLAUDE.md のテスト件数 (Low, 修正済み)

75件 → 54件に更新済み (本チェック時に修正)

### 3. insertHead バッファ容量チェック (Low)

**場所**: `source/parsing/ParserBase.cpp:269`

`SLANG_ASSERT` のみでガード。Release ビルドではアサーション無効。ただし `handleExponentSplit` からの呼び出しは最大約4トークンで、初期容量32に対して十分。実質リスクゼロ。

### 4. macOS の getenv("TERM") 未キャッシュ (Low)

**場所**: `source/util/OS.cpp:194`

`fileSupportsColors()` 呼び出し毎に `std::getenv("TERM")` を実行。起動時に数回呼ばれる程度のため実影響なし。

---

## テスト失敗の内訳 (全54件、全て意図的)

### P1.3: 負のパックド配列範囲チェック (7件)

`source/ast/types/AllTypes.cpp` で `dim.left < 0 || dim.right < 0` を追加。IEEE 1800-2023 準拠の厳密化。

- `$typename` (TypeTests.cpp:866)
- `Array query functions` (ExpressionTests.cpp:743)
- `Array select out of bounds - valid` (ExpressionTests.cpp:1477)
- `Selects with negative bounds` (ExpressionTests.cpp:3044)
- `Non-const name selector` (ExpressionTests.cpp:2149)
- `Complicated lvalue path` (LookupTests.cpp:984)
- `Dynamic array eval` (EvalTests.cpp:510)

### EvalFlags::CovergroupExpr 削除 (20件)

`AllowUnboundedPlaceholder` に置換。covergroup 式内の変数参照が静的評価されなくなった。

- `Covergroup basic errors` (CoverTests.cpp:38)
- `Cover points` (CoverTests.cpp:54)
- `Coverpoint bins` (CoverTests.cpp:108)
- `Non-constant bins expressions` (CoverTests.cpp:166)
- `Covergroup built-in methods` (CoverTests.cpp:192)
- `Covergroup with arguments` (CoverTests.cpp:271)
- `Covergroup formals are const` (CoverTests.cpp:353)
- `Cover cross bin set expressions` (CoverTests.cpp:485)
- `Filter bin selections` (CoverTests.cpp:518)
- `Coverage options` (CoverTests.cpp:552)
- `Coverage system functions` (CoverTests.cpp:596)
- `Coverpoints and cover cross name lookup` (CoverTests.cpp:711)
- `Illegal matches expr` (CoverTests.cpp:739)
- `Cover cross bin set expressions` (CoverTests.cpp:781)
- `v1800-2023: derived covergroups` (SystemFuncTests.cpp:971)
- `v1800-2023: real coverpoint errors` (SystemFuncTests.cpp:1456)
- `v1800-2023: solve-before with reals, array sizes` (EvalTests.cpp:1840)
- `Constraint metadata` (ConstraintTests.cpp:10)
- `JSON dump -- covergroup` (SerializerTests.cpp:531)
- `Sampled value functions` (SystemFuncTests.cpp:839)

### H-02: Enum メソッド評価無効化 (2件)

sukimasim ランタイムが処理するため `nullptr` を返却。

- `Eval enum methods` (EvalTests.cpp:1454)
- `Enum method lookup` (LookupTests.cpp:478)

### H-03: String メソッド評価無効化 (1件)

- `Eval string methods` (EvalTests.cpp:1485)

### Bitstream 切り詰め許容 (3件)

`BadStreamSize` エラーを削除し、サイズ不一致を許容。

- `Streaming operators` (EvalTests.cpp:1657)
- `Streaming concat in non-stream context` (ExpressionTests.cpp:1075)
- `bit-stream cast evaluation` (EvalTests.cpp:1762)

### NewArrayExpression 安全チェック追加 (2件)

`InvalidArraySize` チェックと 16MiB 制限を追加。診断順序が変更。

- `Unused param defaults` (ParameterTests.cpp:486)
- `New array expression` (MemberTests.cpp:37)

### SubroutineSymbols 仮想メソッド処理 (2件)

`MethodFlags::Pure | MethodFlags::Virtual` に変更。DPI 診断の出力順変更。

- `DPI Imports` (SubroutineTests.cpp:200)
- `DPI signature checking` (SubroutineTests.cpp:288)

### CL4: Void ポート + Modport アクセス変更 (3件)

- `User-defined nettype port connection errors` (PortTests.cpp:1398)
- `Multi assign via mutually referential interfaces` (MultiAssignTests.cpp:1085)
- `Invalid nets` (MemberTests.cpp:117)

### PatternContext パーサー変更 (2件)

`parseConditionalStatement()` で `parseExpression()` → `parseSubExpression(ExpressionOptions::PatternContext, 0)` に変更。matches 式サポートのため。

- `Conditional expression inside conditional statement` (StatementParsingTests.cpp:220)
- `Conditional matching expression` (StatementParsingTests.cpp:238)

### 新 AST ノード追加 (1件)

5 syntax kinds, 1 symbol kind, 1 expression kind 追加によるカウント変動。

- `Visit all file` (VisitorTests.cpp:827)

### その他診断順序/件数変更 (残り約11件)

- `Concurrent assertion expression errors` (AssertionTests.cpp:86)
- `Binary operator warnings` (WarningTests.cpp:303)
- `$sformat`, `Format string`, `$typename`, `Distribution functions` (SystemFuncTests.cpp)
- `Class constructor calls` (ClassTests.cpp:268)
- その他 (MemberTests, LookupTests, StatementTests 等)

### CTest 失敗 (4件)

| テスト | 原因 |
|--------|------|
| `unittests` | 上記54件 |
| `regression_all_file` | `all.sv` の covergroup 式が CovergroupExpr 削除で失敗 |
| `regression_cst_json_gen` | `all_file` 依存 |
| `regression_cst_json_roundtrip` | `gen` 出力依存 |

---

## 検証環境

- OS: Linux 6.6.87.2 (WSL2)
- コンパイラ: C++20 対応
- CMake ビルド: デフォルト設定
- 検証日: 2026-03-02

---

Reviewed by: Claude Code (2026-03-02)

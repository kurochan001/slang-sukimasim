# slang-sukimasim コードベース ガイド

## 概要

`slang-sukimasim` は、[MikePopoloski/slang](https://github.com/MikePopoloski/slang) のフォークです。元々は高速で堅牢な SystemVerilog フロントエンド(レキサー、パーサー、型チェック、エラボレーション)を提供します。このフォークは、カスタム SystemVerilog シミュレータ(sukimasim)の開発用に拡張されています。

## 高レベルアーキテクチャ

```
ソースコード(.sv)
     ↓
  ┌──────────────────────────────────────┐
  │      パイプライン構成                 │
  ├──────────────────────────────────────┤
  │ 1. Lexer (字句解析)                  │
  │    - Token, LexerFacts               │
  ├──────────────────────────────────────┤
  │ 2. Preprocessor (プリプロセッサ)     │
  │    - マクロ展開, インクルード処理    │
  │    - プラグマ処理                    │
  ├──────────────────────────────────────┤
  │ 3. Parser (構文解析)                 │
  │    - Concrete Syntax Tree (CST)生成 │
  │    - ParserBase, Parser_*            │
  ├──────────────────────────────────────┤
  │ 4. Syntax Tree (構文木)              │
  │    - SyntaxTree, SyntaxNode          │
  │    - AllSyntax.h (生成ファイル)      │
  ├──────────────────────────────────────┤
  │ 5. Elaboration (エラボレーション)    │
  │    - ASTContext によるセマンティック│
  │    - シンボル解決, 型チェック       │
  │    - Scope, Compilation             │
  ├──────────────────────────────────────┤
  │ 6. Elaborated AST                    │
  │    - Symbol, Expression, Statement   │
  │    - Type, ValueSymbol               │
  ├──────────────────────────────────────┤
  │ 7. Analysis (解析フェーズ)           │
  │    - データフロー解析                │
  │    - 論理解析, クロック推定         │
  └──────────────────────────────────────┘
```

## ディレクトリ構成

### `source/` - メインソースコード

#### `source/parsing/` - 字句解析・構文解析
- **Lexer.cpp**: トークン列生成
- **Preprocessor.cpp**: マクロ展開, インクルード処理
- **Parser.cpp**: メイン構文解析
- **Parser_expressions.cpp**: 式の解析
- **Parser_members.cpp**: メンバー(ポート, パラメータ)の解析
- **Parser_statements.cpp**: ステートメントの解析
- **ParserMetadata.cpp**: パーサーメタデータ
- **Token.cpp, NumberParser.cpp**: ユーティリティ

#### `source/syntax/` - 構文木
- **SyntaxTree.cpp**: メイン入り口
- **SyntaxNode.cpp**: ノード基底クラス
- **SyntaxFacts.cpp**: 構文情報
- **SyntaxVisitor.cpp**: ビジター パターン実装
- **SyntaxPrinter.cpp**: ソースコード再生成

#### `source/ast/` - エラボレートされた AST
- **Compilation.cpp**: 全体のコンパイレーション管理
- **Symbol.cpp**: シンボル基底クラス
- **Expression.cpp**: 式の処理
- **Statement.cpp**: ステートメントの処理
- **Scope.cpp**: スコープ管理
- **ASTContext.cpp**: セマンティック解析コンテキスト
- **Lookup.cpp**: シンボル名前解析

#### `source/ast/symbols/` - シンボル定義
- **InstanceSymbols.cpp**: module, interface インスタンス
- **MemberSymbols.cpp**: module メンバー
- **VariableSymbols.cpp**: 変数(wire, reg, logic等)
- **ParameterSymbols.cpp**: パラメータ
- **PortSymbols.cpp**: ポート定義
- **SubroutineSymbols.cpp**: 関数/タスク
- **ClassSymbols.cpp**: クラス定義
- **BlockSymbols.cpp**: ブロック (always, initial等)
- **CompilationUnitSymbols.cpp**: パッケージ, モジュール定義

#### `source/ast/types/` - 型システム
- **Type.cpp**: 型基底クラス
- **AllTypes.cpp**: スカラー, 配列, 構造体等の型
- **DeclaredType.cpp**: 宣言型, typedef
- **NetType.cpp**: net 型 (wire, wand等)
- **TypePrinter.cpp**: 型の文字列化

#### `source/ast/expressions/` - 式の種類
- リテラル (数値, 文字列等)
- 演算子式
- 関数呼び出し
- キャスト式
- セレクト式 (部分選択, 配列インデックス)
- アサーション式

#### `source/ast/statements/` - ステートメントの種類
- 条件 (if/else, case)
- ループ (for, while, do-while)
- プロシージャルブロック (always, initial)
- その他 (assign, deassign等)

#### `source/ast/builtins/` - ビルトイン機能
- **Builtins.cpp**: ビルトイン型とシステム関数
- UVM パッケージ機能

#### `source/diagnostics/` - 診断・エラー処理
- **DiagnosticEngine.cpp**: 診断エンジン
- **DiagnosticClient.cpp**: 診断クライアント (出力)
- **TextDiagnosticClient.cpp**: テキスト出力
- **JsonDiagnosticClient.cpp**: JSON 出力

#### `source/analysis/` - 後処理解析
- **AnalysisManager.cpp**: 解析管理
- **DataFlowAnalysis.cpp**: データフロー解析
- **ClockInference.cpp**: クロック推定
- **DriverTracker.cpp**: ドライバー追跡

#### `source/driver/` - ドライバー層
- **Driver.cpp**: コマンドライン引数処理, 全体フロー制御
- **SourceLoader.cpp**: ファイル読み込み

#### `source/text/` - テキスト処理
- **SourceManager.cpp**: ソースファイル管理, 位置情報追跡
- **SourceLocation.cpp**: 位置情報
- **Json.cpp**: JSON シリアライゼーション

#### `source/numeric/` - 数値処理
- **SVInt.cpp**: SystemVerilog 整数型
- **ConstantValue.cpp**: 定数値評価
- **Time.cpp**: 時間値

#### `source/util/` - ユーティリティ
- **CommandLine.cpp**: コマンドライン処理
- **BumpAllocator.cpp**: メモリアロケータ
- **TimeTrace.cpp**: パフォーマンス計測

### `include/slang/` - パブリックAPI

source/ と同じ構造をミラーしています。ライブラリユーザーはこちらのヘッダーをインクルードします。

### `tools/` - ツール群

- **driver/** - メインコンパイラ(`slang`)実行ファイル
  - コマンドラインオプション処理
  - AST/CST の JSON シリアライゼーション
  - 診断出力

- **hier/** - 階層構造表示ツール
  - モジュール階層の可視化

- **tidy/** - リント/スタイルチェッカー
  - カスタマイズ可能な設計ルールチェック
  - ANSI ポート宣言, 命名規則等の検査

- **rewriter/** - コード変換ツール
  - 構文木をもとにしたコード再生成/変換

- **reflect/** - リフレクション/型情報抽出ツール
  - データ型の動的検査
  - C++ コード生成

- **threadtest/** - マルチスレッドテスト
  - コンパイレーションの並列処理テスト

### `tests/` - テストスイート

- **unittests/** - ユニットテスト
  - 各コンポーネントの単体テスト

- **regression/** - リグレッションテスト
  - sv-tests (chipsalliance) との互換性テスト

### `docs/` - ドキュメント

- **developer-guide.dox**: 開発者ガイド
- **api-reference.dox**: API リファレンス
- **parsing.dox**: 構文解析フェーズ詳細
- **source-management.dox**: ソース管理
- **diagnostics.dox**: 診断システム
- **tools.dox**: ツール使用法

### `scripts/` - コード生成スクリプト

- **syntax.txt**: 構文定義ファイル
  - AST ノード定義を記述 (AllSyntax.h, SyntaxKind.h が自動生成される)

- **syntax_gen.py**: syntax.txt から C++ コード生成
  - AllSyntax.cpp, SyntaxClone.cpp も生成

- **diagnostics.txt**: 診断定義
  - AllDiags.h と各診断関連ファイルが自動生成

- **diagnostic_gen.py**: diagnostics.txt から診断コード生成

- **tokenkinds.txt, triviakinds.txt**: トークン/トリビア定義

- **systemnames.txt**: システムタスク/関数の名前リスト

## コンパイルパイプライン詳細

### フェーズ 1: Lexing & Preprocessing
```cpp
// SourceLoader がファイルを読み込む
// Lexer がトークンストリーム生成
// Preprocessor が:
//  - マクロ展開
//  - インクルード処理
//  - 条件付きコンパイル
```

### フェーズ 2: Parsing (CST生成)
```cpp
// Parser がトークンストリームから CST (Concrete Syntax Tree) を構築
// CST は元のソース情報を完全に保持 (round-trip可能)
// SyntaxTree オブジェクトにメモリを管理させる
```

### フェーズ 3: Elaboration (AST生成)
```cpp
// Compilation オブジェクトがメイン管理オブジェクト
// CST を走査しながら Symbol を生成
// ASTContext が:
//  - スコープ管理
//  - 型チェック
//  - シンボル解決
//  - エラー/警告発行
// 結果は RootSymbol (設計ルート) に集約される
```

### フェーズ 4: Analysis (オプション)
```cpp
// AnalysisManager が追加解析を実行:
//  - DataFlowAnalysis: 変数の定義/使用追跡
//  - ClockInference: クロック信号推定
//  - DriverTracker: ドライバー信号追跡
```

## 重要なパターンと慣例

### 1. Symbol ベースの設計
- すべての宣言(module, wire, variable等)は **Symbol** の派生クラスとして表現
- Scope が Symbol を管理し、名前ルックアップを提供
- Symbol は SymbolKind enum で分類

### 2. ASTContext による遅延バインディング
- 意図的に多くの解析が遅延される
- Symbol.visit() 呼び出し時に必要な情報を on-demand で計算
- メモリ効率と解析速度のバランス

### 3. Visitor パターン
- ASTVisitor が全 Symbol/Expression/Statement を走査可能
- カスタマイズ可能 (VisitStatements, VisitExpressions フラグ)
- handle() メソッドで特定ノード型をオーバーライド可能

### 4. Diagnostic Engine
- DiagnosticEngine がすべてのエラー/警告を一元管理
- DiagnosticClient で出力形式を切り替え可能 (Text, JSON)
- 診断は scripts/diagnostics.txt で定義される

### 5. スクリプト生成
- AllSyntax.h, SyntaxKind.h, etc は scripts/*.txt から Python スクリプトで生成
- CMake がビルド時に自動実行
- 変更時は CMake を再実行する必要あり

### 6. メモリ管理
- BumpAllocator による高速メモリ確保
- SyntaxTree, Compilation がメモリ所有
- オブジェクトのライフタイムは明確 (通常は同期的)

## コンパイル方法

```bash
# 基本ビルド
cmake -B build
cmake --build build -j32

# テスト実行
ctest --test-dir build --output-on-failure  # すべてのテスト

# ユニットテストのみ実行
./build/bin/unittests

# 単一ユニットテストの実行
./build/bin/unittests "[test-name]"

# リグレッションテスト (sv-tests suite)
# 1037 テスト、約 6-7 分必要、タイムアウト: 20分 (1200秒)
ctest --test-dir build -R regression --output-on-failure

# Python バインディングのビルド
pip install . --no-build-isolation --config-settings build-dir=build/python_build
pytest  # Python テスト実行

# ツール実行
./build/bin/slang [options] file.sv
./build/bin/slang-tidy [options] file.sv
./build/bin/slang-hier [options] file.sv

# CMake ビルドオプション
cmake -B build -DSLANG_INCLUDE_COVERAGE=ON      # カバレッジ有効化
cmake -B build -DSLANG_INCLUDE_PYLIB=ON         # Python バインディング
cmake -B build -DSLANG_INCLUDE_DOCS=ON          # ドキュメント生成
cmake -B build -DBUILD_SHARED_LIBS=ON           # 共有ライブラリ
```

## 主なクラスと関連性

```
Compilation (全体管理)
    ↓
RootSymbol (設計ルート)
    ├── Symbol* members (module, package, etc)
    ├── Type* knownTypes
    └── Scope

Symbol (基底)
    ├── InstanceSymbol (module インスタンス)
    ├── VariableSymbol (wire, variable)
    ├── SubroutineSymbol (function, task)
    ├── PackageSymbol (package)
    └── ... (60+ 派生クラス)

Expression (基底)
    ├── BinaryExpression
    ├── CallExpression
    ├── LiteralExpression
    └── ... (30+ 派生クラス)

Statement (基底)
    ├── BlockStatement
    ├── IfStatement
    ├── ForLoopStatement
    └── ... (15+ 派生クラス)

Type (基底)
    ├── IntegralType (bit, logic, int)
    ├── FloatingType (real, shortreal)
    ├── ArrayType
    ├── StructType
    └── ... (15+ 派生クラス)

ASTContext
    ├── Scope& (現在のスコープ)
    ├── DiagnosticEngine& (エラー報告)
    └── ASTFlags (パース時フラグ)
```

## 拡張点 (sukimasim fork 用)

Phase 1 (DeclarationsAtStart 改善):
- IEEE 1800-2023 関数追加
- 宣言時期の検証強化

Phase 2 (Covergroup 解析):
- 予約キーワード問題修正
- Covergroup パース改善

今後の計画:
- シミュレータ向けの追加セマンティック情報
- 実行時挙動トレース機能

## デバッグのヒント

### 構文木を見る
```cpp
// CST JSON 出力
slang --cst-json output.json file.sv

// AST JSON 出力
slang --ast-json output.json file.sv
```

### 診断メッセージ
- メッセージは scripts/diagnostics.txt で定義
- コード生成は Python スクリプト (diagnostic_gen.py) で自動実行
- 新しい診断を追加するには diagnostics.txt を編集

### シンボルの探索
```cpp
// Compilation.getRoot() で RootSymbol 取得
// Symbol.lookupName() でメンバー検索
// Scope を走査するには iterator をループ
```

## パフォーマンス特性

- **Lexing/Parsing**: O(n) - ファイルサイズに比例
- **Elaboration**: O(n*m) - シンボル数と参照数に依存
- **Analysis**: O(n) - 設計サイズに比例

マルチスレッド対応:
- SLANG_USE_THREADS で有効
- CMake オプション: -DSLANG_USE_THREADS=ON

## よくある操作

### ファイル解析開始
```cpp
Compilation comp;
auto tree = syntax::SyntaxTree::fromFile("file.sv");
comp.addSyntaxTree(tree);
```

### AST アクセス
```cpp
auto root = comp.getRoot();
for (auto symbol : root.members)
    // symbol を処理
```

### エラー報告
```cpp
diagEngine.addDiag(diag::CompilationError, location)
    << "custom message";
```

## コーディングスタイル

- C++20 標準を使用 (C++20 の機能を積極的に活用)
- 関数、パラメータ、ローカル変数は lowerCase 命名
- カラム幅は 100 文字
- `#pragma once` を使用 (ifdef ガード不要)
- `pre-commit` による自動フォーマット:
  ```bash
  pip install pre-commit
  pre-commit install
  ```
- clang-format が自動適用される
- LLVM コーディング標準をベースにしているが、一部例外あり

## Python バインディング

Python から slang ライブラリを使用可能:

```bash
# インストール
pip install pyslang

# ローカルビルドからインストール
pip install . --no-build-isolation --config-settings build-dir=build/python_build

# 使用例
import pyslang
tree = pyslang.SyntaxTree.fromFile('test.sv')
mod = tree.root.members[0]
print(mod.header.name.value)
```

Python バインディングは `bindings/python/` にあり、pybind11 を使用。
公開 API を変更する際は Python バインディングも更新を検討すること。

## 個人設定

- すべての応答は日本語で行うこと
- コード中のコメントは英語で記述
- make / ninja の `-j` オプションは `-j32` でお願いします
- sv-tests などの長時間実行コマンドのタイムアウトは20分(1200秒)
- 1037テスト実行には約6-7分が必要

この CLAUDE.md は、slang-sukimasim コードベースの全体像を把握する最初のステップとしてお使いください。

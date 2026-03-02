# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 概要

`slang-sukimasim` は、[MikePopoloski/slang](https://github.com/MikePopoloski/slang) のフォークです。slang は IEEE 1800-2023 準拠の高速で堅牢な SystemVerilog フロントエンド（レキサー、パーサー、型チェック、エラボレーション）を提供します。このフォークは、カスタム SystemVerilog シミュレータ（sukimasim）の開発用に拡張されています。

## ブランチ運用

| ブランチ | 用途 | テスト状態 |
|----------|------|-----------|
| `master` | upstream (MikePopoloski/slang) との互換性維持 | 全テスト通過 |
| `sukimasim-improvements` | sukimasim シミュレータ開発用 | 54件の意図的差異あり |

**重要**: `sukimasim-improvements` を `master` にマージしないこと。upstream との互換性が失われます。

## ビルドとテスト

```bash
# ビルド
cmake -B build
cmake --build build -j32

# 全テスト実行
ctest --test-dir build --output-on-failure

# ユニットテストのみ
./build/bin/unittests

# 単一テスト実行
./build/bin/unittests "[test-name]"

# リグレッションテスト (sv-tests, 約6-7分, タイムアウト20分)
ctest --test-dir build -R regression --output-on-failure

# ツール実行
./build/bin/slang [options] file.sv
./build/bin/slang-tidy [options] file.sv
./build/bin/slang-hier [options] file.sv
```

### CMake オプション

```bash
cmake -B build -DSLANG_INCLUDE_COVERAGE=ON   # カバレッジ
cmake -B build -DSLANG_INCLUDE_PYLIB=ON      # Python バインディング
cmake -B build -DSLANG_USE_THREADS=ON        # マルチスレッド
```

## 高レベルアーキテクチャ

```
ソースコード(.sv)
     ↓
┌─────────────────────────────────────┐
│ 1. Lexer + Preprocessor             │
│    source/parsing/                  │
├─────────────────────────────────────┤
│ 2. Parser → CST (Concrete Syntax)   │
│    source/parsing/, source/syntax/  │
├─────────────────────────────────────┤
│ 3. Elaboration → AST                │
│    source/ast/                      │
│    - Symbol, Expression, Statement  │
│    - Type checking, Scope resolution│
├─────────────────────────────────────┤
│ 4. Analysis (optional)              │
│    source/analysis/                 │
│    - DataFlow, ClockInference       │
└─────────────────────────────────────┘
```

## 主要ディレクトリ

| ディレクトリ | 内容 |
|-------------|------|
| `source/parsing/` | Lexer, Preprocessor, Parser |
| `source/syntax/` | CST (SyntaxTree, SyntaxNode) |
| `source/ast/` | エラボレートされた AST (Symbol, Expression, Statement, Type) |
| `source/ast/symbols/` | シンボル定義 (60+ クラス) |
| `source/ast/types/` | 型システム (15+ クラス) |
| `source/ast/builtins/` | ビルトイン関数・型 |
| `source/diagnostics/` | エラー/警告処理 |
| `source/analysis/` | データフロー解析等 |
| `include/slang/` | パブリック API ヘッダー |
| `tools/` | slang, slang-tidy, slang-hier 等 |
| `scripts/` | コード生成スクリプト (syntax.txt, diagnostics.txt) |

## 主要クラス

```
Compilation (全体管理)
    └── RootSymbol (設計ルート)
            └── Symbol* members

Symbol (基底) → InstanceSymbol, VariableSymbol, SubroutineSymbol, ...
Expression (基底) → BinaryExpression, CallExpression, LiteralExpression, ...
Statement (基底) → BlockStatement, IfStatement, ForLoopStatement, ...
Type (基底) → IntegralType, FloatingType, ArrayType, StructType, ...

ASTContext: スコープ管理、型チェック、シンボル解決のコンテキスト
```

## コード生成

AST ノードと診断メッセージは `scripts/` 内のテキストファイルから自動生成されます：

- `scripts/syntax.txt` → `AllSyntax.h`, `SyntaxKind.h`
- `scripts/diagnostics.txt` → `AllDiags.h`

変更時は CMake を再実行してください。

## デバッグ

```bash
# CST を JSON 出力
./build/bin/slang --cst-json output.json file.sv

# AST を JSON 出力
./build/bin/slang --ast-json output.json file.sv
```

## コーディングスタイル

- C++20 標準
- 関数、パラメータ、ローカル変数は lowerCase
- カラム幅 100 文字
- `#pragma once` 使用
- `pre-commit install` で clang-format 自動適用

## sukimasim 拡張

このフォークでの主な変更点：

- IEEE 1800-2023 の追加機能サポート
- ImmediateExpectStatement サポート
- 配列メソッドの動作変更
- シミュレータ向けセマンティック情報追加

詳細は `docs/session-*.md` を参照。

## Python バインディング

```bash
pip install . --no-build-isolation --config-settings build-dir=build/python_build
```

```python
import pyslang
tree = pyslang.SyntaxTree.fromFile('test.sv')
```

# slang-sukimasim Rust FFI ハイブリッド化計画

作成日: 2025-01-12
更新日: 2025-01-12

## 概要

slang-sukimasim を FFI ハイブリッドアーキテクチャに移行し、sukimasim 固有の拡張を Rust で実装する。

**目的**:
- シミュレーションコアを Rust で実装し、メモリ安全性を確保
- 今後の sukimasim 固有機能を Rust で開発し、C++ パッチ追加を抑制
- シミュレータと slang フロントエンドの責務を明確に分離

**維持するインターフェース**: C++ API, Python バインディング, AST/CST 構造, JSON 出力

---

## 重要な制約事項

### Rust 化では解決できない問題

**upstream マージの困難さは Rust 化しても変わりません。**

理由：sukimasim-improvements の変更の大部分は slang C++ コア内部の修正であり、Rust ラッパーでは対応できないためです。

### C++ に残さざるを得ない変更（大部分）

| 変更 | 理由 |
|------|------|
| IEEE 1800-2023 パーサー拡張 | CST/AST 構造の変更、Rust では対応不可 |
| ImmediateExpectStatement | パーサー・AST 両方の変更が必要 |
| without/s_without 演算子 | トークン定義・パーサー変更 |
| interface 継承 | エラボレーション処理の変更 |
| parameterized packages | 複雑なシンボル解決 |
| SEGFAULT 修正 | C++ コア内部の修正 |
| diagnostics.txt 変更 | コード生成との整合性 |

### Rust に移動できる変更（少数）

| 変更 | 理由 |
|------|------|
| phase176 (min/max 動作) | 評価結果の後処理として実装可能 |
| `$sukimasim_*` 関数 | 新規システム関数として Rust で実装 |
| カスタム解析・警告 | AST 取得後の処理として実装可能 |

---

## 代替アプローチの比較

| アプローチ | upstream マージ | 工数 | メモリ安全性 |
|-----------|----------------|------|-------------|
| **現状維持** | 困難（変わらず） | なし | なし |
| **upstream への PR** | ✅ 容易になる | 中（レビュー対応） | なし |
| **FFI ハイブリッド** | 困難（変わらず） | 大 | ✅ シミュレータのみ |
| **完全 Rust 書き換え** | N/A（別プロジェクト） | 膨大（5-10人年） | ✅ 全体 |

**推奨**: upstream マージを容易にしたい場合は **upstream への PR** が最も効果的。
FFI ハイブリッドは **シミュレータのメモリ安全性** が主目的の場合に有効。

---

## アーキテクチャ

```
┌─────────────────────────────────────────────────────────┐
│              sukimasim-rs (Rust)                        │
│  ┌───────────────────────────────────────────────────┐  │
│  │  Application Layer - CLI, シミュレーション制御    │  │
│  └───────────────────────────────────────────────────┘  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  Extension Layer - 配列メソッド変更, $sukimasim_* │  │
│  │  ※ 移動可能な変更は少数                          │  │
│  └───────────────────────────────────────────────────┘  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  FFI Binding Layer (autocxx + cxx)               │  │
│  └───────────────────────────────────────────────────┘  │
│  ┌───────────────────────────────────────────────────┐  │
│  │  slang Core (C++) - パーサー, エラボレーション    │  │
│  │  ※ IEEE 1800-2023 拡張等は C++ に残る            │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

---

## 実装フェーズ

### Phase 1: 基盤構築 (1-2 ヶ月)

**目標**: Rust プロジェクト構造と基本 FFI

1. `sukimasim-rs/` ディレクトリ作成
2. Cargo.toml, build.rs 設定 (autocxx)
3. 最小 FFI バインディング:
   - `Compilation` クラス
   - `SyntaxTree::fromFile()`
4. JSON ベースの AST 転送パス実装
5. 簡単な SV ファイルのパース検証

**検証**: Hello World レベルの SV パース → Rust AST 取得

### Phase 2: 拡張レイヤー実装 (2-3 ヶ月)

**目標**: sukimasim 固有機能の Rust 実装

1. 配列メソッドオーバーライド (min/max 動作変更)
2. カスタムシステム関数 `$sukimasim_*`
3. Rust AST 構造の完成
4. AST 後処理ユーティリティ

**検証**: 既存テストとの差異確認

### Phase 3: シミュレーションコア (3-6 ヶ月)

**目標**: Rust シミュレーションエンジン（主要なメリット）

1. イベント駆動エンジン
2. シミュレーション状態管理
3. Python バインディング (PyO3)

**検証**: シミュレーション結果検証

### Phase 4: 統合・最適化 (2-3 ヶ月)

**目標**: 本番運用準備

1. テストスイート統合
2. ドキュメント整備
3. パフォーマンス最適化

---

## ディレクトリ構造

```
slang-sukimasim/
├── source/                     # C++ (IEEE 1800-2023 拡張含む)
├── include/                    # C++ ヘッダー
├── bindings/python/            # pybind11 (維持)
│
└── sukimasim-rs/              # 新規 Rust プロジェクト
    ├── Cargo.toml
    ├── build.rs
    └── src/
        ├── lib.rs
        ├── ffi/               # FFI バインディング
        │   ├── compilation.rs
        │   ├── symbols.rs
        │   └── types.rs
        ├── ast/               # Rust AST 表現
        │   ├── symbol.rs
        │   ├── expression.rs
        │   └── statement.rs
        ├── extensions/        # sukimasim 拡張（少数）
        │   ├── array_methods.rs
        │   └── system_tasks.rs
        ├── simulation/        # シミュレーションコア（主目的）
        │   ├── engine.rs
        │   └── scheduler.rs
        └── python/            # PyO3 バインディング
```

---

## 重要ファイル

| ファイル | 用途 |
|----------|------|
| `include/slang/ast/Compilation.h` | FFI 公開の中心 API |
| `include/slang/ast/ASTSerializer.h` | JSON AST 転送 |
| `source/ast/builtins/ArrayMethods.cpp` | phase176 参照 |
| `bindings/python/pyslang.cpp` | PyO3 統合の参考 |

---

## 検証方法

1. **Phase 1 検証**:
   ```bash
   cd sukimasim-rs
   cargo test ffi_tests
   # 簡単な SV ファイルのパース確認
   ```

2. **Phase 2 検証**:
   ```bash
   cargo test extensions_tests
   # 配列メソッドの動作確認
   ```

3. **Phase 3 検証**:
   ```bash
   cargo test simulation_tests
   # シミュレーション結果の検証
   ```

4. **統合テスト**:
   ```bash
   ctest --test-dir build --output-on-failure
   # 既存 C++ テストとの互換性確認
   ```

---

## リスクと対策

| リスク | 対策 |
|--------|------|
| autocxx カバレッジ不足 | JSON ベースの代替パス |
| slang API 変更 | バージョン固定、変更検知 CI |
| パフォーマンス低下 | ボトルネック計測、最適化 |
| C++ パッチの継続的メンテナンス | upstream PR を検討 |

---

## 期間見積もり

| Phase | 期間 | 内容 |
|-------|------|------|
| 1 | 1-2 ヶ月 | 基盤構築（Rust プロジェクト、基本 FFI） |
| 2 | 2-3 ヶ月 | 拡張レイヤー（配列メソッド、システム関数） |
| 3 | 3-6 ヶ月 | シミュレーションコア（Rust エンジン） |
| 4 | 2-3 ヶ月 | 統合・最適化 |
| **合計** | **8-14 ヶ月** | |

---

## 技術選定

| 項目 | 選定 | 理由 |
|------|------|------|
| FFI | autocxx + cxx | 自動バインディング生成 + 安全性 |
| Python バインディング | PyO3 | Rust との親和性、既存 pybind11 と併用 |
| AST 転送 | JSON ベース | FFI 複雑性軽減、既存 ASTSerializer 活用 |
| ビルドシステム | Cargo + CMake 統合 | 両言語のベストプラクティス |

---

## 結論

### FFI ハイブリッドで得られるもの

- ✅ シミュレーションコアのメモリ安全性
- ✅ 今後の sukimasim 固有機能を Rust で開発可能
- ✅ フロントエンド（slang）とバックエンド（シミュレータ）の責務分離

### FFI ハイブリッドで得られないもの

- ❌ upstream マージの容易化（C++ パッチは残る）
- ❌ 既存 C++ パッチの削減（大部分は C++ に残さざるを得ない）

### 推奨

- **シミュレータのメモリ安全性** が主目的なら → FFI ハイブリッドを進める
- **upstream マージの容易化** が主目的なら → upstream への PR を検討

---

## 参考資料

- [autocxx GitHub](https://github.com/google/autocxx)
- [cxx.rs](https://cxx.rs/)
- [PyO3](https://pyo3.rs/)
- [docs/research-rust-rewrite-2025-01-09.md](./research-rust-rewrite-2025-01-09.md) - Rust 書き換え調査レポート

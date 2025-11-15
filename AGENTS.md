# Repository Guidelines

## プロジェクト構成・モジュール構造
- C++ コアコード: `source/`（`analysis/`, `ast/`, `parsing/`, `util/` など）。
- 公開ヘッダ: `include/` 配下。新しい API も既存の名前空間と配置方針に合わせて追加してください。
- CLI ツール（driver, hier, tidy, reflect など）は `tools/` にあります。
- C++ テストは `tests/unittests/` と `tests/regression/`、Python 用テストは `pyslang/tests` にあります。
- Python バインディング関連は `bindings/python/` および `pyslang/` に配置されています。

## ビルド・テスト・開発コマンド
- 共通ビルド:
  `cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build build -j$(nproc)`
- CMake プリセット例 (Linux):
  `cmake --preset gcc-debug-shared && cmake --build build/gcc-debug-shared`
- C++ テスト実行（ビルドディレクトリから）:
  `cd build && ctest`
- Python バインディングのテスト:
  `pip install -e .[test] && pytest pyslang/tests`

## コーディングスタイル・命名規約
- C++ はスペース 4 つインデント、カラム幅 100。変更したファイルには `.clang-format` を適用してください。
- 型名は PascalCase、関数・メソッドは camelCase を基本とし、意味のある識別子名を付けてください。
- `.clang-tidy` を可能な範囲で利用し、`.editorconfig` に従って改行コードや末尾空白を統一します。
- Python は PEP 8 準拠とし、`.flake8`（`max-line-length = 88`）の設定に合わせてください。

## テスト方針
- 新機能や仕様変更時は `tests/unittests/` に Catch2 ベースの単体テストを追加・更新します。
- バグ再現用の最小 SystemVerilog ケースは `tests/regression/` に追加してください。
- 変更前後で `ctest` が成功すること、および Python を変更した場合は `pytest pyslang/tests` が通ることを確認してください。

## コミット・Pull Request ガイドライン
- コミットメッセージは簡潔な命令形で、必要に応じてスコープを付けます（例: `slang: dedupe bind registrations`）。
- 関連する変更は論理的な単位にまとめ、フォーマット変更だけのコミットは極力分離してください。
- PR には概要、動機、テスト結果、関連 Issue や upstream へのリンクを記載してください。

## エージェント向けメモ
- 自動編集時は対象機能・バグに関連する最小限の差分に留め、無関係な大規模リフォーマットは避けてください。
- 可能な限りコード変更と同時にテストを追加・更新し、挙動の意図をテストで明示してください。

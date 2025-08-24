> **フォーク通知**
> このリポジトリは [MikePopoloski/slang](https://github.com/MikePopoloski/slang) のフォークです。
> 目的：カスタムSystemVerilogシミュレータ（sukimasim）開発のための実験と機能追加。
> 上流同期：upstream/masterと定期的に同期し、可能な限り変更を上流に還元予定。

slang-sukimasim - SystemVerilog言語サービス（フォーク版）
=========================================================
![](https://github.com/MikePopoloski/slang/workflows/CI%20Build/badge.svg)
[![codecov](https://codecov.io/gh/MikePopoloski/slang/branch/master/graph/badge.svg)](https://codecov.io/gh/MikePopoloski/slang)
[![PyPI](https://img.shields.io/pypi/v/pyslang.svg)](https://pypi.org/project/pyslang/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/MikePopoloski/slang/blob/master/LICENSE)

slang-sukimasimは、オリジナルのslangの実験的フォークで、カスタムSystemVerilogシミュレータ開発のための追加機能を含んでいます。slangの高速で堅牢なフロントエンド機能を継承しつつ、シミュレータ実装に向けた独自の改良を含んでいます。

オリジナルのslangは、SystemVerilogコードのレキシング、パージング、型チェック、エラボレーションのための各種コンポーネントを提供するソフトウェアライブラリです。

slangは最速かつ最も準拠性の高いSystemVerilogフロントエンドです（オープンソースの[chipsallianceテストスイート](https://github.com/chipsalliance/sv-tests)による）。

完全なドキュメントはウェブサイトで利用可能です：https://sv-lang.com

### 機能
-   すべてのSystemVerilog機能を完全にパース、解析、エラボレート - 現在のステータスは[このページ](https://sv-lang.com/language-support.html)を参照。
-   ソーステキストがどれほど壊れていても、コンパイルについて堅牢である。これにより、ユーザーがまだ書いている途中でコードが壊れている可能性が高いエディタのハイライトや補完シナリオでコンパイラが使用可能になります。
-   パースツリーは元のソースに往復変換できるため、リファクタリングやコード生成ツールの作成が容易になります。
-   clangのような素晴らしいエラーメッセージを提供。
-   プロダクションスケールのプロジェクトに対して高速で堅牢。

### ユースケース
slangを使用できることの例：
-   非常に高速な構文チェックとリンティングツール
-   プロジェクトのASTをJSONにダンプ
-   含まれているPythonバインディングを介したソースコードの内省
-   SystemVerilogコードの生成とリファクタリング
-   エディタ言語サーバーのエンジンとして
-   下流ツールの前に位置する高速で堅牢なプリプロセッサとして
-   slangをライブラリとして含めることで、合成またはシミュレーションツールのフロントエンドとして

### 上流との差分

このフォークでの主な変更：
- **フェーズ1**：DeclarationsAtStartの改善、IEEE 1800-2023関数の追加
- **フェーズ2**：Covergroup解析の改善（予約語問題の修正）
- **実験的機能**：シミュレータ実装のための内部構造の調査と改良
- **互換性**：上流との完全な互換性を維持（破壊的変更なし）

詳細は[docs/](docs/)ディレクトリのドキュメントを参照してください。

### はじめに

ソースからslangをビルドする手順は[こちら](https://sv-lang.com/building.html)。要約すると：
```
git clone https://github.com/bamba351/slang-sukimasim.git
cd slang-sukimasim
cmake -B build
cmake --build build -j32
```

slangバイナリはすぐにあなたのコードで実行できます。動作の詳細については[ユーザーマニュアル](https://sv-lang.com/user-manual.html)をご覧ください。

slangをライブラリとして使用したい場合は、[開発者ガイド](https://sv-lang.com/developer-guide.html)をお読みください。

### 試してみる

パージング、型チェック、エラー検出を[ウェブ上](https://sv-lang.com/explore/)でライブで実験できます（Matt Godboltの優れた[Compiler Explorer](https://godbolt.org/)に触発されました）。

### Pythonバインディング

このプロジェクトには、PyPI経由でインストールできるライブラリのPythonバインディングも含まれています：
```
pip install pyslang
```
または、インストール済みのバージョンを最新リリースに更新するには：
```
pip install -U pyslang
```
または、ローカルビルドをチェックアウトしてインストールするには：
```
git clone https://github.com/bamba351/slang-sukimasim.git
cd slang-sukimasim
pip install .
```

#### Python使用例

'test.sv'ソースファイルが与えられた場合：
```sv
module memory(
    address,
    data_in,
    data_out,
    read_write,
    chip_en
  );

  input wire [7:0] address, data_in;
  output reg [7:0] data_out;
  input wire read_write, chip_en;

  reg [7:0] mem [0:255];

  always @ (address or data_in or read_write or chip_en)
    if (read_write == 1 && chip_en == 1) begin
      mem[address] = data_in;
  end

  always @ (read_write or chip_en or address)
    if (read_write == 0 && chip_en)
      data_out = mem[address];
    else
      data_out = 0;

endmodule
```

slangを使用して構文ツリーをロードして検査できます：
```py
import pyslang

tree = pyslang.SyntaxTree.fromFile('test.sv')
mod = tree.root.members[0]
print(mod.header.name.value)
print(mod.members[0].kind)
print(mod.members[1].header.dataType)
```

```
memory
SyntaxKind.PortDeclaration
reg [7:0]
```

任意のSystemVerilog式を評価することもできます：
```py
session = pyslang.ScriptSession()
session.eval("logic bit_arr [16] = '{0:1, 1:1, 2:1, default:0};")
result = session.eval("bit_arr.sum with ( int'(item) );")
print(result)
```

```
3
```

### 上流同期

```bash
# upstreamの設定（初回のみ）
git remote add upstream https://github.com/MikePopoloski/slang.git

# upstreamとの同期
git fetch upstream --prune --tags
git switch master && git merge upstream/master
git push origin master
```

upstream/masterと定期的に同期されます。このフォークからの汎用的な改善は上流へのPRが検討されます。

### 連絡先とサポート

- **フォーク固有の問題**：このリポジトリのIssuesに登録
- **slangコアの問題**：[上流リポジトリ](https://github.com/MikePopoloski/slang)に登録
- **シミュレータ実装に関する質問**：このリポジトリのDiscussionsを使用

バグレポート、コメント、提案、ドキュメントの改善、プルリクエストによる本格的な新機能など、どのような形でも貢献を歓迎します。

### ライセンスとクレジット

このフォークは、オリジナルのslangと同じMITライセンスで提供されます。

#### オリジナル作者
>   Copyright (c) 2015-2025 Michael Popoloski

#### フォークの修正
>   Copyright (c) 2025 bamba351 (sukimasim fork)

slangのオリジナル作者であるMichael Popoloski氏と、slangコミュニティのすべての貢献者に特別な感謝を。

---

**オリジナルライセンス（MIT）：**

>   Copyright (c) 2015-2025 Michael Popoloski
>
>   Permission is hereby granted, free of charge, to any person obtaining a copy
>   of this software and associated documentation files (the "Software"), to deal
>   in the Software without restriction, including without limitation the rights
>   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
>   copies of the Software, and to permit persons to whom the Software is
>   furnished to do so, subject to the following conditions:
>
>   The above copyright notice and this permission notice shall be included in
>   all copies or substantial portions of the Software.
>
>   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
>   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
>   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
>   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
>   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
>   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
>   THE SOFTWARE.
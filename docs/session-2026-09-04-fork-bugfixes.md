# sukimasim 起点のフォーク修正 3 件と基準リスト更新 (2026-09-01〜09-04)

upstream マージ ([session-2026-08-26-upstream-merge.md](session-2026-08-26-upstream-merge.md))
の後、`../sukimasim` (gitlab kurochan001/sukimasim) の issue から slang フォーク側に
原因があると判明した 3 件を修正した。本書はその記録と、**意図的差異の基準リストを
76 件から 67 件へ更新した** 記録である。

## 前提: sukimasim との関係

- `../sukimasim` は本フォークを `../slang-sukimasim` (branch `sukimasim-improvements`)
  として CMake で参照する。sukimasim 側の issue 番号 (#1334 など) は gitlab の番号
- sukimasim の issue が slang 側の問題だった場合、修正はこちらのブランチにコミットし、
  コミット題名に `(sukimasim #NNNN)` を付ける
- 修正後は必ず `./build/bin/unittests` を回し、後述の基準リストと突き合わせる

## 修正一覧

### 4e72616bf (2026-09-01) extends 節の未宣言基底クラス名を報告する (sukimasim #1349)

- 症状: `class B extends Undeclared;` が 0 エラーで通過
- 原因: `handleExtends` は再入エラボレーション対策で先に `baseClass` をエラー型の
  番兵にセットする。基底名の非修飾ルックアップがクラス自身のスコープを通る際、
  「基底がエラー型なら未宣言を抑制する」規則 (`LookupResultFlags::SuppressUndeclared`)
  が番兵と本物のエラー基底を区別できず、未宣言名を握りつぶしていた
- 修正: `ClassType::isBaseClassPending` で番兵状態を明示的に追跡し、自身の基底を
  解決中は抑制をスキップ (`include/slang/ast/symbols/ClassSymbols.h`,
  `source/ast/Lookup.cpp`, `source/ast/symbols/ClassSymbols.cpp`)
- テスト: 失敗 76 件で基準と一致 (変化なし)

### c369ce280 (2026-09-04) パックド配列の負の境界を許容する (sukimasim #1236)

- 症状: `logic [-1:4] b` (IEEE 1800-2023 §6.9.1 の例そのもの) が 1 ビットに潰され、
  格納値が壊れる
- 原因: フォーク独自の「P1.3」ゲートが `PackedArrayType::fromDim` で負の境界を
  `InvalidPackedRange` 警告 + `{0,0}` の 1 ビットプレースホルダに置き換えていた。
  ゲートの当初の動機は CVA6 の `config_pkg::cva6_cfg_empty` (ゼロ初期化パラメータで
  `[XLEN-1:0]` が `[-1:0]` になる) を落とさず通すこと
- 修正: ゲートを削除し、upstream 同様に負の境界をそのまま通す
  (`source/ast/types/AllTypes.cpp`)。`tests/unittests/ast/ExpressionTests.cpp` の
  期待値も upstream 相当に戻した
- **コミット本文が空**なので根拠はソース内コメントと本書のみ
- テスト: この修正で範囲選択・負境界系の 9 件が新たに合格 (後述)

### 4fed723b5 (2026-09-04) $rtoi を符号付き飽和 int64 経由で変換する (sukimasim #1334)

- 症状: UBSan ビルドで `$rtoi(-2.9)` が未定義動作を報告
  (sukimasim の `tests/bugs/test_issue841_rtoi_truncate.sv`,
  `tests/combination/test_comb_math_handlers.sv`)
- 原因: `RtoIFunction::eval` が負の double を直接 `uint64_t` にキャストしていた。
  x86-64 では偶然正しいビット列になるため非サニタイザビルドでは露見しない
- 修正: `std::trunc` でゼロ方向に切り捨て (IEEE 1800-2023 §20.5)、NaN/無限大は 0、
  int64 範囲外は飽和、int64 を経由して `SVInt(32, ...)` へ
  (`source/ast/builtins/ConversionFuncs.cpp`)
- **このファイルは upstream と byte 単位で同一**なので、同じ修正が upstream にも
  当てはまる (PR 未送付、候補)
- テスト: 失敗 67 件、すべて基準内、新規なし

## 基準リストの更新: 76 件 → 67 件

`4fed723b5` 時点で `./build/bin/unittests` は 2493 件中 **67 件失敗**
(assertions 22785 中 179 失敗)。旧基準 76 件との差分:

- 新規失敗: **なし**
- 新たに合格した 9 件 (c369ce280 の負境界ゲート削除による):

```
$typename
Array query functions
Array select out of bounds - invalid
Array select out of bounds - valid
No range select ordering error for single bit value
Range order mismatch error suppressed in untaken conditionals
Range select out of bounds during constant eval
Selects with negative bounds
bit-stream cast evaluation
```

**運用ルール**: 失敗件数が動いたら (fail→pass の改善でも) 同じコミットで本書の
基準リストと CLAUDE.md の件数を更新する。c369ce280 / 4fed723b5 はこれを怠り、
本書で後追い更新した。

### 突き合わせ手順

旧文書の `-r xml | grep TestCase` は全テスト名を出すだけで失敗を抽出できない。
以下を使うこと:

```bash
./build/bin/unittests -r junit 2>/dev/null > /tmp/junit.xml
python3 - <<'PY'
import re, xml.etree.ElementTree as ET
fail = {tc.get('name') for tc in ET.parse('/tmp/junit.xml').getroot().iter('testcase')
        if tc.find('failure') is not None or tc.find('error') is not None}
doc = open('docs/session-2026-09-04-fork-bugfixes.md').read()
sec = doc[doc.rindex('### 現在の基準リスト'):]  # rindex: この手順自身にも同じ文字列がある
base = {l.strip() for l in re.search(r'```(.*?)```', sec, re.S).group(1).splitlines() if l.strip()}
print('fail', len(fail), 'base', len(base))
print('NEW (退行):', sorted(fail - base))
print('NOW PASSING (基準更新が必要):', sorted(base - fail))
PY
```

### 現在の基準リスト (67 件, `4fed723b5` 時点)

```
$sformat invalid %p call
Accessing program objects from modules is disallowed
Additional explicit port expression checks
Additional implicit port type mismatch checking
Annex D option system tasks and functions
Ansi port initializers
Array reduction method errors
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
Non-ansi port errors
Non-const name selector
Out-of-bounds range selects in consteval
Sampled value functions
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
foreach loop with function call as array name
v1800-2023: derived covergroups
v1800-2023: real coverpoint errors
v1800-2023: solve-before with reals, array sizes
```

## 未検証・未対応 (次回への申し送り)

- **`diag::InvalidPackedRange` が dead になった**: c369ce280 で唯一の発行箇所が消え、
  `scripts/diagnostics.txt` と生成物にしか残っていない。upstream マージ時の
  「取り残した dead code」と同じパターン。診断定義を削るか、意図的に残すか要判断
- **CVA6 は未検証**: P1.3 ゲートの動機だった `cva6_cfg_empty` 経由の `[-1:0]` は
  今後、警告なしに 2 ビット型として通る。`../CVA6` で elaboration が変わっていないか
  未確認
- **リグレッションスイート (`ctest -R regression`) は 3 件とも未実行**。
  単体テストのみで確認した
- **4fed723b5 は upstream PR 候補** (ファイルが upstream と同一)

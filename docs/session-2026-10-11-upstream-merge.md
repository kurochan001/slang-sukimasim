# upstream v12.0 の取り込みと基準リスト更新 (2026-10-11)

`upstream/master` (MikePopoloski/slang) を `sukimasim-improvements` にマージした記録。
前回の取り込みは [session-2026-08-26-upstream-merge.md](session-2026-08-26-upstream-merge.md)、
直前の基準リストは [session-2026-09-04-fork-bugfixes.md](session-2026-09-04-fork-bugfixes.md)
(67 件)。本書で **基準リストを 67 件 → 71 件に更新した**。

## 取り込み範囲

| 項目 | 値 |
|------|-----|
| マージコミット | `01f828f34` |
| 前回の共通祖先 | `a5ab85255` (2026-08-26) |
| upstream HEAD | `55e72bcfd` (2026-10-10) |
| コミット数 | 61 (v12.0 リリースを含む) |
| 差分 | 174 ファイル / +7835 / -1083 |

`master` は upstream の純粋な祖先だったので、チェックアウトせずに
`git fetch upstream master:master` で fast-forward した (694 コミット)。

## テキスト衝突の解決 (3 ファイル / 7 ハンク)

### `source/ast/Compilation.cpp` (1)

`errorMissingDef` の条件。フォーク側の `isUninstantiatedIgnoringLint()` を維持し、
upstream の波括弧付き整形を採用した。`isUninstantiated()` は LintMode で一律 true に
なるため、`--enable-uvm` 実行の全スコープでこの診断が消える (sukimasim #1200)。

### `source/driver/CompatSettings.cpp` (2)

upstream `f780b8c6b` がモード別の重大度リストを `diagnostics.txt` の
`compat <mode> <severity> = { ... }` 構文へ移管した。upstream 側を全面採用し、
フォークの `CompatMode::Strict` の昇格を `compat Strict error` ブロックとして
`scripts/diagnostics.txt` に移した。中身は Default のリスト + `missing-time-scale`
で、これは旧 C++ コードで Strict が Default 分岐にフォールスルーしていたときの
重大度マップと等価。`toString(CompatMode::Strict)` は `SLANG_ENUM` が識別子を
そのまま文字列化するので `"Strict"` となり、生成テーブルのキーと一致する
(`build/source/DiagCode.cpp` で確認済み)。

### `source/ast/builtins/StringMethods.cpp` (4)

upstream が `atoi` / `atoreal` / `realtoa` の定数評価を実装した
(`1c475726f`, `9ad44fd1f`)。フォークの Phase 2.1 は文字列メソッドを実行時エンティティ
として扱い `eval()` を `return nullptr` にしているので、フォーク側を維持した。
ただし `StringAtoIMethod` のシグネチャは upstream の `int` → `LiteralBase` 変更を
採用している (登録側の呼び出しが `LiteralBase::Decimal` などを渡すようになったため)。
引数は未使用のまま。

## テキスト衝突しなかったが壊れた箇所

### `ClassSymbols`: `specializationDepth` の二重定義

upstream `e3412c632` (#1943) とフォーク `4ae5c0d07` (sukimasim #1410) が、
**独立に同じ問題** (遅延クラス特殊化の無限連鎖によるスタックオーバーフロー) を解決し、
**どちらもカウンタを `specializationDepth` と命名していた**。自動マージは両方の
メンバ宣言を残し、`ClassType` に同名フィールドが 2 つある非コンパイル状態になった。

2 つのガードは測るものが違う:

| | upstream | フォーク |
|---|---|---|
| 数えるもの | **同一**ジェネリッククラスの連鎖の深さ | **任意の**囲むクラス特殊化の連鎖の深さ |
| 上限 | `maxRecursiveClassSpecialization` (既定 8) | `maxClassSpecializationDepth` (既定 128) |
| 診断 | `RecursiveClassSpecialization` | `MaxClassSpecializationDepth` |
| 位置 | キャッシュ検索の**前** (生成時に打ち切る) | キャッシュ検索の**後** (§8.25 の型同一性を保つ) |

フィールドを分離した (`specializationDepth` / `dependencyDepth`)。1 本に統合すると
フォーク側の広いカウントが upstream のガードを踏み、**互いに無関係なジェネリック
クラスを 9 段ネストしただけで** `RecursiveClassSpecialization` を誤報する。

なお `maxClassSpecializationDepth` と `MaxClassSpecializationDepth` はフォークの
公開オプション・公開診断で、名前を検査するテストが 3 つある。upstream のガードで
置き換えることはできない。

### `source/analysis/TautologicalCompare.cpp`: `-Wswitch`

upstream `68fe49ac4` の新しいトートロジー比較チェックは `ExpressionKind` を
網羅的に switch する。フォークは `ExpressionKind::Matches` (`expr matches pattern`,
IEEE 1800-2023 §12.6) を追加しているので 2 箇所が `-Wswitch` で警告した
(**Apple clang の `-Werror` ではエラー**)。`matches` は成功時にパターン変数を
束縛するので非 pure、リテラルでもないとして両 switch に追加した (`558efc5a2`)。

### `#1410` のテストが upstream のガードに先取りされる

`maxClassSpecializationDepth` を設定するフォークのテスト 2 件は
`maxRecursiveClassSpecialization` を既定の 8 のままにしていた。upstream のガードが
先に走るため、`MaxClassSpecializationDepth` の代わりに
`RecursiveClassSpecialization` が出る (あるいは有限連鎖がエラーになる)。
テスト側で `maxRecursiveClassSpecialization` を引き上げ、各テストが名乗っている
機構だけを検査するようにした (`9e965faf8`)。ガード本体は変更していない。

## DPI export は無修正で両立

upstream `2a721f722` は `checkDPIMethods` 内に `noteReference(sub)` を追加し、
フォーク `cce923a81` の `resolveDPIExports()` はその `checkDPIMethods({})` を呼ぶ。
別領域なので衝突せず、結果として `resolveDPIExports()` 経由でも export が
used 扱いになる (望ましい方向)。

## テスト結果

```
./build/bin/unittests
  test cases: 2546 | 2475 passed | 71 failed
  assertions: 22984 | 22739 passed | 245 failed

ctest --test-dir build
  FAILED: unittests (上記 71 件)
  FAILED: regression_cst_json_roundtrip  (既知の未修正バグ, 2026-08-26 から継続)
```

(`ast_lit_tests` はマージ直後は失敗していたが、後述の `ValueMustBePositive` /
`IndexValueInvalid` の `error` 復帰 (同日) で解消した。)

**基準リストからの退行は 0 件。** 新規失敗 4 件はすべて、既存のフォーク意図的差異を
upstream の新規テストが初めて踏んだもの:

| 新規失敗 | 帰属 |
|---|---|
| `Eval atoreal and realtoa` | Phase 2.1: 文字列メソッドを定数評価しない。upstream `9ad44fd1f` の新テスト |
| `Eval string to integer conversions` | 同上 (`atoi` 系, upstream `1c475726f`) |
| `JSON DiagnosticClient` | フォークは `UsedBeforeDeclared` のメッセージに `(IEEE 1800-2023 §6.20.1)` を付ける。upstream `f9bd6fcb0` の新テストが文言を完全一致で検査する |
| `Effective width of masks, shifts, division, and concatenations` | フォークは宣言初期化子以外の代入 (`assign` / `always_comb`) で幅警告を出さない。upstream `801f5bf32` の新テストが `assign` を使う。同じ差異は既に別テストの期待値を 0 件に書き換える形でベースライン化されている (`WarningTests.cpp` の `test3`) |

### `ast_lit_tests`: `ValueMustBePositive` / `IndexValueInvalid` を upstream の `error` に戻した (解消)

upstream `ea4f84c55` が追加した `tests/regression/ast/assignment-pattern-diags.sv` が
`ValueMustBePositive` (4 箇所) と `IndexValueInvalid` (1 箇所) を注釈するが、
フォークは `scripts/diagnostics.txt` でこの 2 つを `error` → `warning` に降格していた
(マージ前 `cce923a81` の時点で既にそう)。既定の警告セットに入らないため lit の
既定起動では出力されず、注釈と食い違っていた。

降格の出自は `d2353ce9b` (2026-03-14, OpenTitan/Ibex DV 対応) で、同コミットは
`ValueMustNotBeUnknown` / `InvalidPackedRange` / `CannotIndexScalar` /
`SelectEndianMismatch` も一緒に降格している。今回は **依頼どおり
`ValueMustBePositive` と `IndexValueInvalid` の 2 件だけを upstream の
`error <Name> "<msg>"` 形式に戻した** (警告はオプション名を要求するが、`error` は
取らないので `value-must-be-positive` / `index-value-invalid` というオプション名も
同時に消える)。この 2 つのオプション名は警告グループにも `compat` ブロックにも
現れていなかったので、参照は孤立しない。

結果:

- `ast_lit_tests` → **Passed**
- `unittests` の失敗集合は **71 件のまま、名前も完全一致** (NEW 0 / FIXED 0)。
  `diag::ValueMustBePositive` / `diag::IndexValueInvalid` を検査するテストは
  重大度ではなく diag code を見ているので影響を受けない

残る注意点 (今回は変えていない):

- `SelectExpressions.cpp` の X/Z スキップ (`d2353ce9b`) はそのままなので、
  `IndexValueInvalid` が出るのは **定数として確定した不正インデックス**だけ。
  それが警告ではなくハードエラーになり、CLI の終了コードが非 0 になる
- 上記の兄弟 4 件 (`ValueMustNotBeUnknown` など) と `AllTypes.cpp` の
  プレースホルダ型は降格のまま
- `source/diagnostics/DiagCode.cpp` と `source/diagnostics/slang/diagnostics/*.h`
  はコミット済みだが CMake は `build/source/` 側を生成・コンパイルするため
  **ビルドには使われていない死んだ生成物**。今回も更新していない

なお **`ast_lit_tests` と `regression_cst_json_roundtrip` は ctest 上の 1 テストで、
基準リスト (unittests のテストケース名の列挙) には現れない**。ctest 側の差異は
今後もこの散文の側に記録する。

## 現在の基準リスト (71 件, `9e965faf8` 時点)

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
Effective width of masks, shifts, division, and concatenations
Enum method eval with non-const args
Enum method hierarchical reference errors
Enum method lookup
Eval atoreal and realtoa
Eval enum methods
Eval string methods
Eval string to integer conversions
Format string - errors
Implicit net creation with missing identifier in port connection -- GH #1888
Invalid continuous assign
Invalid name component lookup handling
Invalid nets
Invalid unpacked dimensions
JSON DiagnosticClient
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

## 未検証・申し送り

- **macOS / Apple clang の `-Werror` は未検証** (Linux/gcc のみ)。`-Wswitch` は
  修正済みだが、upstream 61 コミット分の新規コードに他の警告が残る可能性がある
- `regression_cst_json_roundtrip` は 2026-08-26 から未修正。再現手順と修正案は
  [session-2026-08-26-upstream-merge.md](session-2026-08-26-upstream-merge.md)
- ~~`ast_lit_tests` の `ValueMustBePositive` / `IndexValueInvalid` 降格~~ →
  2026-10-11 に upstream の `error` へ戻して解消。同じ `d2353ce9b` が降格した
  `ValueMustNotBeUnknown` / `InvalidPackedRange` / `CannotIndexScalar` /
  `SelectEndianMismatch` は手付かず。OpenTitan/Ibex DV と CVA6 が本当にこの緩和を
  必要としているかは引き続き未確認
- `compat Strict` ブロックは Default のリストを丸ごと複製している。upstream が
  モード間の継承構文を導入したら差分を縮められる
- Phase 2.1 (文字列メソッドの定数評価無効化) は upstream が定数評価を実装した今、
  差分が広がり続ける。sukimasim 側が実行時評価を本当に必要とするか再確認の価値あり
- `master` (`55e72bcfd`) と `sukimasim-improvements` は 2026-10-11 に origin へ push 済み

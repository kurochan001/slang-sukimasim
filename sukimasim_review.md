# sukimasim review

対象: `sukimasim-improvements` の HEAD（commit `e857d494a`）。

## Findings (severity順)
- [Medium] `$abs` が `RealMath1Function` で登録されているため real のみ受け付けます。LRM 上で `$abs` が整数/ビットベクトルにも許可される場合、型チェックで弾かれる／戻り型が期待とずれる可能性があります。`source/ast/builtins/MathFuncs.cpp:335`
- [Medium] `scripts/systemnames.txt` で `$abs` を追加したのに、チェックイン済み生成ヘッダ `slang/parsing/KnownSystemName.h` が未更新です（`Abs` が無い／`KnownSystemName_traits` の件数も旧値）。生成物を配布・利用する構成だと `$abs` が解釈不能になり得ます。`scripts/systemnames.txt:58-61`, `slang/parsing/KnownSystemName.h:60-75`, `slang/parsing/KnownSystemName.h:384-387`
- [Low] covergroup に formal 引数がある場合、`new` はその引数にのみバインドされ、インスタンス名の文字列引数は許可されません。LRM が `new("name", ...)` を許可する場合は挙動差分になります。`source/ast/expressions/AssignmentExpressions.cpp:746-766`
- [Low] 変更点に対する回帰テストが不足しています。`$abs` のパース／型チェック（real/整数）や、covergroup `new` の引数パターンをカバーするテストが未追加です。`source/ast/builtins/MathFuncs.cpp:335`, `source/ast/expressions/AssignmentExpressions.cpp:746-766`

## Questions / Assumptions
LRM 準拠の期待を確認したいです。(1) `$abs` は整数・ビットベクトル入力も許容する想定でしょうか？ (2) covergroup の `new` で instance 名引数が formal 引数と併存できる想定でしょうか？

## Change Summary
`$abs` を既存の math 系 system function 群に追加し、covergroup `new` の引数バインドを formal 引数に従う形へ変更しています。

## Suggested Tests
`tests/unittests/ast/SystemFuncTests.cpp` などに `$abs` の real/整数ケースを追加し、`tests/unittests/ast/CoverTests.cpp` へ `new("name", ...)` と `new(arg1, ...)` の許可／拒否を明示するテストを追加すると安全です。

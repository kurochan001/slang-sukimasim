# Covergroup制限の詳細分析結果

作成日: 2025-08-24
分析者: sukimasim開発チーム

## 🎯 重要な発見

### "3+ bins制限"は誤解だった

**調査結果**: slangには「3つ以上のbinsを定義できない」という制限は**存在しない**。

実際の問題は、SystemVerilogの予約語を誤ってbin名として使用していたことによるパースエラーだった。

## 📊 テスト結果

### ❌ 失敗するケース（予約語使用）
```systemverilog
covergroup cg;
    coverpoint value {
        bins small = {[0:3]};   // ERROR: "small"は予約語
        bins medium = {[4:7]};  // ERROR: "medium"は予約語  
        bins large = {[8:15]};  // ERROR: "large"は予約語
    }
endgroup
```

### ✅ 成功するケース（予約語回避）
```systemverilog
covergroup cg;
    coverpoint value {
        bins low = {[0:3]};     // OK
        bins mid = {[4:7]};     // OK  
        bins high = {[8:15]};   // OK
    }
endgroup
```

### ✅ 11個のbins定義も問題なし
```systemverilog
covergroup cg;
    coverpoint value {
        bins bin0 = {0};
        bins bin1 = {1};
        bins bin2 = {2};
        bins bin3 = {3};
        bins bin4 = {4};
        bins bin5 = {5};
        bins bin6 = {6};
        bins bin7 = {7};
        bins bin8 = {8};
        bins bin9 = {9};
        bins bin10 = {10};
    }
endgroup
```

## 🔍 SystemVerilog予約語（ドライブ強度関連）

以下の予約語はbin名として使用できない：
- `small` (SmallKeyword)
- `medium` (MediumKeyword)
- `large` (LargeKeyword)
- `weak0`, `weak1`
- `strong0`, `strong1`
- `pull0`, `pull1`
- `highz0`, `highz1`

## 📝 影響を受けるテストケース

sukimasimのCovergroup関連テストで失敗している8件は、以下の2つのカテゴリに分類される：

1. **予約語使用によるパースエラー**（推定4-5件）
   - bin名に予約語を使用しているケース
   - 簡単な名前変更で解決可能

2. **実際の制限**（推定3-4件）
   - `get_coverage()`メソッド未認識
   - `binsof`/`intersect`構文の複雑なケース

## 🚀 対応方針

### Phase 2-A: テストケース修正（即対応可能）
- 予約語を使用しているテストケースのbin名を変更
- 例: `medium` → `mid`, `small` → `low`, `large` → `high`

### Phase 2-B: 実際の制限への対応
1. **get_coverage()メソッド実装**
   - CoverageSymbols.cppでメソッド認識を追加
   
2. **binsof/intersect構文改善**
   - より複雑な構文パターンのサポート

## 📊 修正後の期待効果

- **Covergroup成功率**: 69.4% → 85%以上（予約語問題解決）
- **実質成功率**: 99.0% → 99.3%（+6件改善）

## 🔧 次のステップ

1. sukimasimのCovergroup失敗テストを精査
2. 予約語使用箇所を特定して修正版作成
3. 実際のslang制限（get_coverage等）の対応検討

---
*分析完了: 2025-08-24*
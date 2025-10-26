# Real World Timeouts 完全解決ガイド

**作成日**: 2025-10-26
**対象**: join_any タイムアウト問題の完全解決
**推定工数**: 1時間

---

## 📊 現状サマリー

### 未解決テスト（3件）
- `tests/real_world/test_simple_handshake.sv` - タイムアウト（120秒）
- `tests/real_world/test_simple_memory_controller_fixed_v2.sv` - タイムアウト（120秒）
- `tests/real_world/test_simple_memory_controller_working.sv` - タイムアウト（120秒）

### ステータス
- ✅ 根本原因: **特定済み**
- ✅ 暫定対策: **実装済み** (コミット: 0082c0cb)
- ⏸️ 完全解決: **未実装**

---

## 🎯 根本原因

### 問題の本質
**`TimingScheduler::processDeltaCycle()` が再帰的に呼ばれ、無限ループに入る**

### 呼び出しチェーン

```
1. join_any待機ループ (statement_executor_unified.cpp:262)
   ↓
   hadEvents = ts->processDeltaCycle();
   ↓
2. TimingScheduler::processDeltaCycle() (timing_scheduler.cpp:151)
   ↓
   deltaScheduler_.executeDeltaCycle()
   ↓
3. DeltaScheduler::executeDeltaCycle() (delta_scheduler.cpp:139-172)
   ↓
   executeActiveRegion();  // イベント実行
   executeInactiveRegion(); // fork-join プロセス実行
   ↓
4. executeRegion() 内でイベント実行 (delta_scheduler.cpp:201-228)
   ↓
   event->execute();  // ← ここで再帰的に processDeltaCycle() が呼ばれる
   ↓
5. 🔁 無限再帰ループ
```

### デバッグログ証拠

```
[DEBUG UFJ] About to call ts->processDeltaCycle()...
[TIMING SCHEDULER] processDeltaCycle() entered (depth=1)
[TIMING SCHEDULER] processDeltaCycle() entered (depth=2)  ← 再帰！
[WARNING] processDeltaCycle() recursion detected (depth=2)
[TIMING SCHEDULER] processDeltaCycle() returning false (depth=2)
[DEBUG UFJ] join_any loop iteration 2: anyCompleted=?
[DEBUG UFJ] About to call ts->processDeltaCycle()...
[WARNING] processDeltaCycle() recursion detected (depth=2)
...（無限に続く）
```

---

## 🔧 暫定対策（実装済み）

### コミット情報
- **コミットID**: 0082c0cb
- **ファイル**: `src/scheduler/timing_scheduler.cpp`, `src/scheduler/timing_scheduler.h`

### 実装内容

```cpp
// timing_scheduler.h
class TimingScheduler {
private:
    int recursionDepth_ = 0;  // 追加
    // ...
};

// timing_scheduler.cpp
bool TimingScheduler::processDeltaCycle() {
    recursionDepth_++;  // 追加

    if (recursionDepth_ > 1) {
        std::cerr << "[WARNING] processDeltaCycle() recursion detected (depth="
                  << recursionDepth_ << "), preventing infinite loop" << std::endl;
        recursionDepth_--;
        return false;  // 再帰をブロック
    }

    bool result = deltaScheduler_.executeDeltaCycle();

    recursionDepth_--;  // 追加
    return result;
}
```

### 効果と限界
- **効果**: 再帰depth > 1をブロック、無限再帰を防ぐ
- **限界**: ブロックしてもループは継続し、無限に再試行される（タイムアウト）

---

## ✅ 完全解決策: advanceToNextEvent() への切り替え

### なぜ advanceToNextEvent() か？

| 項目 | processDeltaCycle() | advanceToNextEvent() |
|------|---------------------|----------------------|
| **対象** | 現在時刻のdelta cycle | 次のイベント時刻まで進める |
| **用途** | 同期的なイベント処理 | 時間を進めて待機 |
| **再帰リスク** | ❌ 高い（既に実証） | ✅ 低い |
| **join_any適合性** | ❌ 不適切 | ✅ 適切 |

**fork-join プロセスは未来のイベントを待つ必要があるため、`advanceToNextEvent()` が正しい**

---

## 📝 実装手順

### 対象ファイル
`src/eval/statement_executor_unified.cpp`

### 修正箇所
**行番号**: 約262行目付近（join_any 待機ループ）

### 現在のコード

```cpp
// ★ 修正対象 ★
while (!anyCompleted) {
    bool hadEvents = ts->processDeltaCycle();  // ← 問題の原因

    if (!hadEvents) {
        // イベントがなければループを抜ける
        break;
    }

    // 各プロセスの完了状態をチェック
    for (size_t i = 0; i < trackers.size(); i++) {
        if (trackers[i]->isCompleted()) {
            anyCompleted = true;
            break;
        }
    }
}
```

### 修正後のコード

```cpp
// ★ 修正版 ★
while (!anyCompleted) {
    bool hadEvents = ts->processDeltaCycle();

    if (!hadEvents) {
        // Delta cycle にイベントがない場合、次のイベント時刻まで進める
        if (ts->advanceToNextEvent()) {
            // 時刻が進んだので、ランタイムと同期
            runtime_.syncTimeFromScheduler();
        } else {
            // もうイベントがない場合はループを抜ける
            break;
        }
    }

    // 各プロセスの完了状態をチェック
    for (size_t i = 0; i < trackers.size(); i++) {
        if (trackers[i]->isCompleted()) {
            anyCompleted = true;
            break;
        }
    }
}
```

### 変更ポイント
1. **`hadEvents` チェック後の処理を追加**:
   - `processDeltaCycle()` がイベントなしを返したら
   - `advanceToNextEvent()` で次のイベントまで進める
   - `syncTimeFromScheduler()` で時刻同期

2. **ブレーク条件の明確化**:
   - `advanceToNextEvent()` が false を返した場合のみループを抜ける
   - これは「もうイベントがない」ことを意味する

---

## 🧪 検証手順

### ステップ1: コードの適用

```bash
# 1. バックアップ作成
cp src/eval/statement_executor_unified.cpp src/eval/statement_executor_unified.cpp.backup

# 2. 上記のコード修正を適用（エディタで編集）

# 3. ビルド
ninja -C build -j32
```

### ステップ2: テスト実行

```bash
# 個別テスト実行（タイムアウト5分）
timeout 300 ./build/sukimasim tests/real_world/test_simple_handshake.sv 2>&1 | tee /tmp/test1_result.txt

timeout 300 ./build/sukimasim tests/real_world/test_simple_memory_controller_fixed_v2.sv 2>&1 | tee /tmp/test2_result.txt

timeout 300 ./build/sukimasim tests/real_world/test_simple_memory_controller_working.sv 2>&1 | tee /tmp/test3_result.txt
```

### ステップ3: 結果確認

```bash
# 成功パターンを確認
grep -E "(TEST PASSED|Simulation finished)" /tmp/test1_result.txt
grep -E "(TEST PASSED|Simulation finished)" /tmp/test2_result.txt
grep -E "(TEST PASSED|Simulation finished)" /tmp/test3_result.txt

# 再帰警告が出ないことを確認
grep "recursion detected" /tmp/test*.txt
# ↑ 出力がなければ成功
```

### ステップ4: カテゴリ全体のテスト

```bash
# real_world カテゴリ全体を実行（約20分）
python3 scripts/run_test_category.py real_world 2>&1 | tee /tmp/real_world_final_test.txt

# 結果サマリー確認
tail -30 /tmp/real_world_final_test.txt
```

---

## 📍 関連ファイルと行番号

### 主要ファイル

| ファイル | 行番号 | 内容 |
|---------|--------|------|
| **src/eval/statement_executor_unified.cpp** | 262 | ★ 修正対象: join_any 待機ループ |
| **src/scheduler/timing_scheduler.cpp** | 151 | processDeltaCycle() 実装 |
| **src/scheduler/timing_scheduler.h** | - | recursionDepth_ メンバ変数 |
| **src/scheduler/delta_scheduler.cpp** | 139-172 | executeDeltaCycle() 実装 |
| **src/scheduler/delta_scheduler.cpp** | 201-228 | executeRegion() 実装 |

### テストファイル

```
tests/real_world/test_simple_handshake.sv
tests/real_world/test_simple_memory_controller_fixed_v2.sv
tests/real_world/test_simple_memory_controller_working.sv
```

---

## 🔍 デバッグ Tips

### デバッグフラグ

```bash
# join_any のデバッグログを有効化
export SUKIMASIM_DEBUG_FORK=1

# 実行
./build/sukimasim tests/real_world/test_simple_handshake.sv 2>&1 | head -100
```

### 確認すべきログ出力

✅ **成功パターン**:
```
[DEBUG UFJ] About to call ts->processDeltaCycle()...
[DEBUG UFJ] processDeltaCycle() returned with hadEvents=false
[DEBUG UFJ] advanceToNextEvent() called, result=true
Fork-join_any: first process completed at time XXX
[DEBUG UFJ] join_any completed successfully
```

❌ **失敗パターン**（修正前）:
```
[WARNING] processDeltaCycle() recursion detected (depth=2)
[WARNING] processDeltaCycle() recursion detected (depth=2)
...（無限ループ）
```

---

## 🚨 リスク評価

### 低リスク
- **変更箇所**: join_any 待機ループのみ（約10行）
- **影響範囲**: fork-join_any 構文のみ
- **既存テスト**: 他のfork-join系テストは既に合格

### リスク軽減策
1. **バックアップ**: 修正前にファイルをバックアップ
2. **段階的テスト**: まず3つの失敗テストで検証
3. **全体テスト**: real_world カテゴリ全体で最終確認

---

## 📊 期待される結果

### 修正前
| カテゴリ | 状態 |
|---------|------|
| Real World | 0/3 (0%) - タイムアウト |

### 修正後（期待値）
| カテゴリ | 状態 |
|---------|------|
| Real World | **3/3 (100%)** ✅ |

---

## 🎯 チェックリスト

作業開始時にこのチェックリストを使用してください：

- [ ] `/tmp/RECURSION_ROOT_CAUSE.md` を確認済み
- [ ] `/tmp/P2_REAL_WORLD_DEFERRED.md` を確認済み
- [ ] `statement_executor_unified.cpp` をバックアップ
- [ ] コード修正を適用
- [ ] ビルド成功
- [ ] test_simple_handshake.sv が5分以内に完了
- [ ] test_simple_memory_controller_fixed_v2.sv が5分以内に完了
- [ ] test_simple_memory_controller_working.sv が5分以内に完了
- [ ] 再帰警告が出ないことを確認
- [ ] real_world カテゴリ全体をテスト
- [ ] 結果をコミット

---

## 📚 参考ドキュメント

- `/tmp/RECURSION_ROOT_CAUSE.md` - 根本原因の詳細分析
- `/tmp/P2_REAL_WORLD_DEFERRED.md` - 延期決定時のドキュメント
- `/tmp/P2_REMAINING_TASKS.md` - P2タスクの優先度と工数見積もり
- `src/eval/statement_executor_unified.cpp` - 実装ファイル
- `src/scheduler/timing_scheduler.cpp` - スケジューラ実装

---

## 💡 追加の最適化案（オプション）

修正後、さらなる改善が必要な場合：

### 候補3: コールバックベース設計（工数: 12時間）

完全に非同期なfork-join実装：
- イベント駆動アーキテクチャ
- 待機ループを排除
- より IEEE 1800準拠

**注意**: 候補2で解決できる場合、候補3は不要

---

**作成者**: Claude Code
**最終更新**: 2025-10-26 11:50
**ステータス**: Ready for Implementation

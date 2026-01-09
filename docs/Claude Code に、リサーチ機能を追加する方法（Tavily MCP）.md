# Claude Code に、リサーチ機能を追加する方法（Tavily MCP）

## はじめに

Claude.ai には Web検索/リサーチ機能が用意されていて非常に便利です（プランや設定に依存）。
では、Claude Codeにも同様の機能があるのでしょうか？

**結論：Claude Code には内蔵の WebSearch ツールがありますが、利用不可になる場合があります。**

より安定した高機能なウェブ検索を実現するには、MCP（Model Context Protocol）サーバーを追加することをお勧めします。本記事では、最も手軽な方法である **Tavily MCP** の導入手順を解説します。

## Claude Code 内蔵 WebSearch vs Tavily MCP

| 項目 | Tavily MCP | Claude Code 内蔵 WebSearch |
|------|------------|---------------------------|
| **安定性** | ✅ 独立 API で安定 | ⚠️ 利用不可の場合あり |
| **検索エンジン** | Tavily AI 検索エンジン | Claude の標準検索 |
| **結果数** | 最大 20 件指定可能 | 固定（約 10 件） |
| **追加機能** | extract, crawl, map | 検索のみ |
| **フィルタリング** | ドメイン指定、日付範囲、トピック | ドメイン指定のみ |
| **コンテンツ取得** | 生コンテンツ取得オプション | サマリーのみ |
| **地域指定** | 国別ブースト可能 | 米国のみ |
| **API 制限** | 無料: 月 1,000 リクエスト | Claude 利用制限に含まれる |

**推奨**: 安定性と機能性から **Tavily MCP** の使用をおすすめします。

## 内蔵 WebSearch が利用不可になるケース

Claude Code の内蔵 WebSearch は、以下の状況で利用不可（`unavailable` エラー）になることがあります：

### 1. 地域制限

:::note alert
**重要**: 内蔵 WebSearch は **米国のみ** で利用可能です。
:::

日本を含む米国以外の地域からアクセスした場合、WebSearch は利用できません。これが最も一般的な利用不可の原因です。

### 2. レート制限

| エラーコード | 説明 |
|-------------|------|
| `too_many_requests` | API レート制限を超過 |
| `max_uses_exceeded` | リクエストあたりの最大検索回数を超過 |

短時間に多くの検索を実行すると、レート制限に達することがあります。

### 3. 使用量制限

- WebSearch の使用は **日次の利用制限** にカウントされます
- 無料プランでは特に制限が厳しくなります
- 利用制限に達すると、リセットまで待つ必要があります

### 4. 内部エラー・接続問題

| エラーコード | 説明 |
|-------------|------|
| `unavailable` | 内部エラーが発生 |
| `invalid_input` | 無効な検索クエリ |
| `query_too_long` | クエリが最大長を超過 |

接続状況によって検索の可用性が変わる場合があります。

### 5. サンドボックス・ネットワーク制限

Claude Code のサンドボックス設定やネットワーク制限により、外部ドメインへのアクセスがブロックされる場合があります。

### なぜ Tavily MCP が推奨されるのか

| 問題 | 内蔵 WebSearch | Tavily MCP |
|------|---------------|------------|
| 地域制限 | ❌ 米国のみ | ✅ 世界中で利用可能 |
| レート制限 | Claude の制限に依存 | ✅ 独立した API 制限 |
| 可用性 | 不安定な場合あり | ✅ 安定した独立サービス |

**結論**: 米国以外から使用する場合、または安定した検索機能が必要な場合は、**Tavily MCP の導入を強く推奨** します。

## Tavily MCPとは

Tavilyは、LLM向けに最適化されたWeb検索APIを提供するサービスです。Tavily MCPを使うことで、Claude Codeから直接Web検索や情報抽出が可能になります。

### 主な機能

- **tavily-search**: Web検索（ニュース、一般検索、期間指定など）
- **tavily-extract**: 指定URLからコンテンツを抽出
- **tavily-crawl**: サイト内クロール（リンク追跡してコンテンツ収集）
- **tavily-map**: サイト構造のマッピング

## セットアップ手順

### 1. Tavily APIキーの取得

1. [tavily.com](https://tavily.com) にアクセス
2. アカウントを作成（GoogleアカウントやGitHubアカウントでサインアップ可能）
3. ダッシュボードからAPIキーをコピー

:::note info
**無料枠について**
- 月1,000 API credits が無料  
- クレジットカード登録不要
:::

### 2. Claude CodeにMCPサーバーを追加

ターミナルで以下のコマンドを実行します：

```bash
claude mcp add tavily -e TAVILY_API_KEY=tvly-xxxxxxxx -- npx -y tavily-mcp@latest
```

`tvly-xxxxxxxx` の部分を、取得した実際のAPIキーに置き換えてください。

:::note info
`claude mcp add` は `--scope local|user|project` を指定できます。
`project` を選ぶとプロジェクト直下の `.mcp.json` に保存されます。
デフォルトはローカル（`~/.claude.json` のプロジェクト設定）に保存されます。
:::

設定は `~/.claude.json` に以下のように保存されます：

```json
{
  "projects": {
    "/path/to/your/project": {
      "mcpServers": {
        "tavily": {
          "type": "stdio",
          "command": "npx",
          "args": ["-y", "tavily-mcp@latest"],
          "env": {
            "TAVILY_API_KEY": "tvly-xxxxxxxx"
          }
        }
      }
    }
  }
}
```

### 3. 動作確認

Claude Codeを再起動し、以下のように質問してみましょう：

```
最新のSystemVerilog関連のニュースを検索して
```

Tavilyが正しく設定されていれば、Web検索結果を元に回答が返ってきます。

## 使用例

### 技術調査

```
SUNDIALS 7.xの最新リリース情報を調べて
```

### ニュース検索

```
過去7日間のAI半導体関連ニュースを検索して
```

### 特定サイトからの情報抽出

```
https://example.com のページ内容を抽出して要約して
```

## 各ツールの詳細パラメータ

### tavily-search

| パラメータ | 説明 | デフォルト |
|-----------|------|-----------|
| `query` | 検索クエリ（必須） | - |
| `max_results` | 最大結果数 | 10 |
| `search_depth` | 検索深度: basic, advanced, fast, ultra-fast | basic |
| `topic` | トピック: general, news | general |
| `include_raw_content` | 生コンテンツを含める | false |
| `include_images` | 画像を含める | false |
| `days` | 日数（news トピック時） | 3 |
| `include_domains` | 含めるドメイン | [] |
| `exclude_domains` | 除外するドメイン | [] |
| `country` | 国別ブースト（小文字英語） | "" |

### tavily-extract

| パラメータ | 説明 | デフォルト |
|-----------|------|-----------|
| `urls` | URL のリスト（必須） | - |
| `extract_depth` | 抽出深度: basic, advanced | basic |
| `include_images` | 画像を含める | false |

### tavily-crawl

| パラメータ | 説明 | デフォルト |
|-----------|------|-----------|
| `url` | 開始 URL（必須） | - |
| `max_depth` | 最大クロール深度 | 1 |
| `max_breadth` | 各ページから辿るリンク数 | 20 |
| `limit` | 処理する最大リンク数 | 50 |
| `instructions` | クロール指示（自然言語） | - |

### tavily-map

| パラメータ | 説明 | デフォルト |
|-----------|------|-----------|
| `url` | 開始 URL（必須） | - |
| `max_depth` | 最大マッピング深度 | 1 |
| `max_breadth` | 各ページから辿るリンク数 | 20 |
| `limit` | 処理する最大リンク数 | 50 |

## 他の選択肢

Tavily以外にも、リサーチ機能を追加する方法があります：

### mcp-omnisearch

複数の検索エンジンを統合したMCPサーバーです。

```json
{
  "mcpServers": {
    "mcp-omnisearch": {
      "type": "stdio",
      "command": "npx",
      "args": ["-y", "mcp-omnisearch"],
      "env": {
        "TAVILY_API_KEY": "",
        "BRAVE_API_KEY": "",
        "PERPLEXITY_API_KEY": ""
      }
    }
  }
}
```

利用したいサービスのAPIキーだけ設定すればOKです。

:::note info
これらはコミュニティ提供のMCPサーバーです。導入前にソース確認を推奨します。
:::

### Claude-Deep-Research

学術的なリサーチに特化したMCPサーバーで、DuckDuckGoとSemantic Scholarを統合しています。APA形式の引用も自動生成されます。

:::note info
こちらもコミュニティ提供のサーバーです。利用は自己責任で。
:::

## セキュリティ上の懸念事項

Tavily MCPを導入する前に、以下のセキュリティリスクを理解しておく必要があります。

### 1. APIキーの管理

```bash
# この設定はローカルの設定ファイルに平文で保存される
claude mcp add tavily -e TAVILY_API_KEY=tvly-xxxxxxxx -- npx -y tavily-mcp@latest
```
:::note warn
**リスク**
- APIキーが `~/.claude/settings.json` / `.claude/settings.json` / `.claude/settings.local.json` に平文で保存される  
- MCP を **project scope** で追加した場合、`.mcp.json` に設定が書き込まれる  
- Gitリポジトリに誤ってコミットしてしまう可能性がある  
- 他のユーザーやプロセスから読み取られる可能性がある
:::

**対策**
- `.gitignore` に設定ファイルを追加する
- 環境変数経由でAPIキーを渡す場合は、`.env` ファイルも `.gitignore` に含める
- 定期的にAPIキーをローテーションする
- Tavilyダッシュボードで使用量を監視する

### 2. 検索クエリの外部送信

Tavily MCPを使用すると、検索クエリがTavilyのサーバーに送信されます。

:::note alert
**注意が必要なケース**
- プロジェクト名や内部コードネームを含む検索  
- 未公開の技術情報に関する検索  
- 顧客情報や機密データを含むクエリ
:::

**例：避けるべき検索クエリ**
```
# NG: 社内プロジェクト名が外部に送信される
「ProjectX の暗号化アルゴリズムの脆弱性について調べて」

# OK: 一般的な技術用語のみ
「AES-256 の既知の脆弱性について調べて」
```

**対策**
- 機密情報を含まない一般的な技術用語で検索する
- 社内プロジェクト固有の情報は検索クエリに含めない
- 企業のセキュリティポリシーを確認する

### 3. 取得コンテンツの信頼性

Web検索結果は必ずしも正確とは限りません。

**リスク**
- 古い情報や誤った情報が含まれる可能性
- SEO対策された低品質なコンテンツが上位に来る場合がある
- 悪意のあるコードサンプルが含まれる可能性

**対策**
- 重要な技術情報は公式ドキュメントで裏取りする
- コードサンプルは内容を理解してから使用する
- 複数のソースで情報を確認する

### 4. サードパーティMCPサーバーのリスク

Tavily MCPは `npx` 経由でnpmパッケージとして実行されます。

:::note warn
**考慮すべき点**
- npmパッケージはサプライチェーン攻撃のリスクがある  
- パッケージの更新により意図しない動作変更が起こる可能性  
- `@latest` 指定は常に最新版を取得するため、破壊的変更の影響を受ける
:::

**対策**
```bash
# バージョンを固定する（推奨）
claude mcp add --transport stdio tavily-mcp \
  --env TAVILY_API_KEY=tvly-xxxxxxxx \
  -- npx -y mcp-remote@<version> "https://mcp.tavily.com/mcp?api_key=${TAVILY_API_KEY}"
```

- 信頼できるパッケージのみを使用する（GitHubスター数、メンテナンス状況を確認）
- バージョンを固定して、意図しない更新を防ぐ
- 可能であればソースコードをレビューする

### 5. 企業環境での利用

企業でClaude Codeを使用する場合、追加の考慮が必要です。

| 確認項目 | 内容 |
|----------|------|
| 情報セキュリティポリシー | 外部APIへのデータ送信が許可されているか |
| データ分類 | 検索に使用する情報の機密レベル |
| 監査要件 | API使用ログの保持が必要か |
| ネットワーク制限 | 外部APIへのアクセスが許可されているか |

**対策**
- IT部門やセキュリティチームに事前確認する
- 個人の開発マシンでのみ使用し、CI/CD環境では無効化する
- 必要に応じてプロキシ経由でアクセスする

### セキュリティチェックリスト

導入前に以下を確認してください：

- [ ] APIキーが `.gitignore` 対象のファイルに保存されている
- [ ] 機密情報を含む検索クエリを送信しないルールを理解している
- [ ] 企業のセキュリティポリシーを確認した（企業利用の場合）
- [ ] パッケージのバージョンを固定した
- [ ] Tavilyダッシュボードで使用量モニタリングを設定した

## 補足：Claude Agent SDK

Anthropicは **Claude Agent SDK** を公式に提供しており、  
コーディング用途だけでなく、汎用的なエージェント構築用途を想定しています。

- ビジネス支援エージェント
- リサーチ支援エージェント
- ワークフロー自動化エージェント

つまり、Claude Codeの基盤（エージェントハーネス）は汎用的なエージェント構築に適しており、リサーチ用途も公式に想定されています。

## まとめ

| 項目 | 内容 |
|------|------|
| ビルトイン機能 | WebSearch あり（ただし利用不可の場合あり） |
| 推奨方法 | Tavily MCP |
| 無料枠 | 月1,000 API credits |
| 設定の手軽さ | コマンド1行 |
| 利用可能ツール | search, extract, crawl, map |

Claude Codeでの開発作業中に技術情報を調べたい場面は多いと思います。Tavily MCPを追加しておけば、ターミナルを離れることなく安定したWeb検索ができるようになり、開発効率が向上します。

内蔵の WebSearch が利用不可になった場合でも、Tavily があれば安心です。月1,000 API credits の無料枠があれば、日常的な技術調査には十分でしょう。ぜひ試してみてください。

## 参考リンク

- [Tavily公式サイト](https://tavily.com)
- [Tavily MCP GitHub](https://github.com/tavily-ai/tavily-mcp)
- [Tavily MCPドキュメント](https://docs.tavily.com/documentation/mcp)
- [Anthropic - Building agents with the Claude Agent SDK](https://www.anthropic.com/engineering/building-agents-with-the-claude-agent-sdk)

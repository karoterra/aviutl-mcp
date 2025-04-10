# AviUtl MCP

[AviUtl](https://spring-fragrance.mints.ne.jp/aviutl/) を
[MCP (Model Context Protocol)](https://modelcontextprotocol.io/)
を通して操作できるようにするためのソフトウェア群です。

## 動作環境

以下の AviUtl 環境を前提とします。

- AviUtl 1.10
- 拡張編集 0.92

また、本ソフトウェアの動作の為に以下が必要です。

- Microsoft Visual C++ 再頒布可能パッケージ
- [uv](https://docs.astral.sh/uv/)
- お好きな MCP ホスト (Claude Desktop など)

## 使い方

1. `mcp.auf` を AviUtl の `plugins` フォルダに配置する
2. MCP ホスト (Claude Desktop など) で AviUtl MCP の MCP サーバーを設定する
3. AviUtl を起動し MCP ホストから操作する

### Claude Desktop の設定

MCP ホストの設定方法の例として Claude Desktop から使えるようにするための設定方法を示します。

1. `%APPDATA%\Claude\claude_desktop_config.json` に以下の内容を追加してください。
   `C:\\path\\to` の部分は各自の環境に合わせて適宜変更してください。

    ```json
    {
        "mcpServers": {
            "aviutl_mcp": {
                "command": "uv",
                "args": [
                    "--directory",
                    "C:\\path\\to\\aviutl-mcp\\mcp_server",
                    "run",
                    "mcp",
                    "run",
                    "C:\\path\\to\\aviutl-mcp\\mcp_server\\main.py"
                ]
            }
        }
    }
    ```

2. Claude Desktop を再起動します。

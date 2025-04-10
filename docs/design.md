# Design Document

## 概要

本ソフトウェアは AviUtl プラグイン (C++) と MCP サーバー (Python) の 2 つから構成されます。

```mermaid
flowchart LR
    subgraph AviUtl MCP
    Plugin["`AviUtl プラグイン`"]
    Server["`MCP サーバー`"]
    end
    Client["`MCP ホスト
    MCP クライアント`"]

    Client <-->|MCP| Server
    Server <-->|名前付きパイプ| Plugin
```

AviUtl プラグイン自体に MCP サーバーの機能をまとめていない理由としては以下が挙げられます。

- 開発を開始した時点で C/C++ 用の MCP SDK が公開されていなかったた
- MCP サーバーを C/C++ で実装するのは大変そう
- MCP サーバー機能をプラグインの外に出したほうがプラグインのバイナリサイズを減らせそう

一方で、この構成によるデメリットとしては以下が考えられます。

- MCP サーバーのセットアップがユーザーにとって分かりにくい
  - MCP サーバーを `.exe` にして配布することができれば解消できるか？

## ディレクトリ構成

- `bin/` : 動作確認用の AviUtl を置く場所
- `docs/` : 設計に関するメモ
- `mcp_server/` : MCP サーバー (Python)
  - `.venv/` : Python 仮想環境
- `plugin/` : AviUtl プラグイン (C++)
  - `build/` : プラグインのビルド出力先
  - `src/` : ソースコード
  - `vendor/` : 外部ライブラリ

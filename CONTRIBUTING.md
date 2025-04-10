# Contribution Guide

## 開発環境のセットアップ

1. 必要なツールをインストールします。
   - CMake
   - Git
   - MSVC (Visual Studio 2022)
   - uv
2. リポジトリをクローンします。
```
git clone --recursive <repository>
```
3. プラグインをビルドします。
```
cd aviutl-mcp/plugin
cmake --preset default

# デバッグビルド
cmake --build --preset debug
# リリースビルド
cmake --build --preset release
```
4. Pythonの仮想環境を作成し、依存関係をインストールする
```
uv sync --frozen
```

## コードスタイル

C/C++ コードは ClangFormat を使用してフォーマットしてください。

Python コードは Ruff を使用してフォーマットしてください。

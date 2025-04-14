import json
from collections.abc import AsyncIterator
from contextlib import asynccontextmanager
from dataclasses import dataclass
from pathlib import Path

from aviutl_mcp_server.assets import AssetManager
from aviutl_mcp_server.exo import create_exo
from aviutl_mcp_server.plugin_ipc import send_message_to_plugin
from mcp.server.fastmcp import Context, FastMCP


@dataclass
class AppContext:
    asset_manager: AssetManager


@asynccontextmanager
async def app_lifespan(server: FastMCP) -> AsyncIterator[AppContext]:
    """アプリケーションのライフサイクル管理"""
    # アプリケーションの初期化
    asset_path = Path("./assets")
    asset_manager = AssetManager(asset_path)
    try:
        yield AppContext(asset_manager=asset_manager)
    finally:
        # クリーンアップ処理
        pass


mcp = FastMCP(
    "aviutl_mcp",
    instructions="MCP server for video editing software AviUtl",
    lifespan=app_lifespan,
)


@mcp.tool()
def get_available_nodes(ctx: Context) -> str:
    """AviUtlでオブジェクトを追加する際に利用可能なノードの情報を取得する"""
    asset_manager: AssetManager = ctx.request_context.lifespan_context.asset_manager
    nodes = asset_manager.param_node_assets.assets
    node_info = [{"name": node.name, "description": node.description} for node in nodes]
    return json.dumps(node_info, indent=4, ensure_ascii=False)


@mcp.tool()
def get_node_info(ctx: Context, node_name: str) -> str:
    """AviUtlのノードの情報を取得する

    Args:
        node_name (str): ノード名

    Returns:
        str: ノードの情報
    """
    asset_manager: AssetManager = ctx.request_context.lifespan_context.asset_manager
    node = asset_manager.param_node_assets.get_asset(node_name)
    if node is None:
        return f"Node '{node_name}' not found."
    return node.get_mcp_resource()


@mcp.tool()
def get_project_info(ctx: Context) -> str:
    """AviUtlのプロジェクト情報を取得する"""
    response = send_message_to_plugin({"id": 1, "method": "get_project_info"})
    if "error" in response:
        return f"Error: {response['error']}"
    return json.dumps(response, indent=4, ensure_ascii=False)


@mcp.tool()
def add_object(ctx: Context, frame: int, layer: int, object: dict) -> str:
    """タイムラインにオブジェクトを追加する

    Args:
        frame (int): オブジェクトの先頭のフレーム番号。0でシーンの先頭。
        layer (int): レイヤー番号 (0 - 99)
        object (dict): オブジェクトの情報

    Example:
        `object` には以下のような情報を指定します。
        `frame_range` には1以上の整数による配列を指定してください。
        `frame_range` の要素数は2以上で、最初の要素が最小値、最後の要素が最大値になります。
        `frame_range` の要素数が3以上の場合、最初の要素と最後の要素以外はキーフレームとして扱われます。
        利用可能なノードは `get_available_nodes` で取得できます。
        ノードの詳細は `get_node_info` で取得できます。
        ```json
        {
            "overlay": true,
            "camera": false,
            "audio": false,
            "frame_range": [1, 101],
            "nodes": [
                {
                    "name": "テキスト",
                    "params": {
                        "テキスト": "テスト",
                        "文字色": [255, 0, 0]
                    },
                    "trackbars": {
                        "サイズ": 100
                    }
                },
                {
                    "name": "標準描画",
                    "trackbars": {
                        "X": 0,
                        "Y": 0
                    }
                }
            ]
        }
        ```

        `trackbars` の各値に数値を指定するとその値で変化しません。
        `trackbars` の各値に以下のような辞書を指定すると値を変化させることができます。
        ```json
        {
            "X": {
                "points": [-200, 200],
                "type": "Linear",
                "accelerate": false,
                "decelerate": false,
                "parameter": null
            },
        }
        ```
        `points` には `frame_range` の要素数と同じ数の数値による配列を指定してください。
        `type` には以下のいずれかを指定してください。
        - Linear
        - Acceleration
        - Curve
        - Teleportation
        - Ignore_midpoint
        - Move_certain_amount
        - Random
        - Repetition
        `accelerate` には真偽値を指定してください。
        `decelerate` には真偽値を指定してください。
        `parameter` には整数を指定してください。
    """
    exopath = Path(__file__).absolute().parent / "outputs" / "mcp.exo"
    asset_manager: AssetManager = ctx.request_context.lifespan_context.asset_manager
    exo = create_exo(object, asset_manager)
    with open(exopath, "w", encoding="cp932") as f:
        exo.dump(f)
    response = send_message_to_plugin(
        {
            "id": 2,
            "method": "add_object",
            "params": {"frame": frame, "layer": layer, "filename": str(exopath)},
        }
    )
    if "error" in response:
        return f"Error: {response['error']}"
    return json.dumps(response, indent=4, ensure_ascii=False)


if __name__ == "__main__":
    mcp.run(transport="stdio")

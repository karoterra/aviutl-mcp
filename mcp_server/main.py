import json
from contextlib import closing

import pywintypes
import win32file
from mcp.server.fastmcp import FastMCP

PIPE_NAME = r"\\.\pipe\aviutl_mcp_pipe"

mcp = FastMCP("aviutl_mcp")


def send_message_to_plugin(message: dict) -> dict:
    handle = None
    try:
        # パイプを開く
        handle = win32file.CreateFile(
            PIPE_NAME,
            win32file.GENERIC_READ | win32file.GENERIC_WRITE,
            0,
            None,
            win32file.OPEN_EXISTING,
            0,
            None,
        )
        with closing(handle):
            # メッセージをエンコード
            msg_str = json.dumps(message).encode("shift_jis")
            msg_size = len(msg_str)

            # メッセージのサイズと内容を送信
            win32file.WriteFile(handle, msg_size.to_bytes(4, "little"))
            win32file.WriteFile(handle, msg_str)

            # レスポンスのサイズを受信
            result, size_data = win32file.ReadFile(handle, 4)
            response_size = int.from_bytes(size_data, "little")

            # レスポンスの内容を受信
            result, data = win32file.ReadFile(handle, response_size)
            response = json.loads(data.decode("shift_jis"))

            return response

    except pywintypes.error as e:
        if handle:
            win32file.CloseHandle(handle)
        return {"error": str(e)}


@mcp.tool()
def get_project_info() -> str:
    """AviUtlのプロジェクト情報を取得する"""
    response = send_message_to_plugin({"id": 1, "method": "get_project_info"})
    if "error" in response:
        return f"Error: {response['error']}"
    return json.dumps(response, indent=4, ensure_ascii=False)


if __name__ == "__main__":
    mcp.run(transport="stdio")

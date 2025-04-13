from pathlib import Path
from typing import List

import exolib
import yaml  # type: ignore

from aviutl_mcp_server.types import AssetType


class ExoColor(exolib.Color):
    """exolib.Color の拡張

    exolib.Color のコンストラクタをカスタムする
    """

    def __init__(self, color):
        """コンストラクタ

        Parameters
        ----------
        color : list or ExoColor
            RGB値を持つリストまたは ExoColor インスタンス
        """
        if isinstance(color, ExoColor):
            red = color.red
            green = color.green
            blue = color.blue
        elif isinstance(color, list):
            if len(color) != 3:
                raise ValueError("RGB values must be a list of three integers.")
            red, green, blue = color
        else:
            raise ValueError("RGB values must be a list of three integers.")
        super().__init__(red, green, blue)

    def __repr__(self):
        return f"ExoColor(red={self.red}, green={self.green}, blue={self.blue})"


class DynamicObjectParamNode(exolib.ObjectParamNode):
    """transformatino_table を動的に変更可能な ObjectParamNode"""

    def __init__(self, transformation_table: dict, **params):
        self.transformation_table = transformation_table
        super().__init__(**params)


class ParamNodeItemDefinition:
    """param_node アイテム定義

    param_node アセットの params, trackbars の定義を表すクラス
    """

    def __init__(self, definition: dict):
        self.name = str(definition.get("name", ""))
        self.exo_name = str(definition.get("exo_name", ""))
        self.param_type = self.get_item_type(definition.get("type", ""))
        self.description = definition.get("description", "")
        self.default = definition.get("default", None)

    def __repr__(self) -> str:
        return f"ParamNodeItemDefinition(name={self.name}, exo_name={self.exo_name}, param_type={self.param_type})"

    @staticmethod
    def get_item_type(type_name: str) -> type:
        """アイテムの型を取得する"""
        match type_name:
            case "Color":
                return ExoColor
            case "Text":
                return exolib.Text
            case "String":
                return exolib.String
            case "Float":
                return exolib.Float
            case "Int":
                return exolib.Int
            case "Boolean":
                return exolib.Boolean
            case "IntTrackBar":
                return exolib.IntTrackBarRanges
            case "FloatTrackBar":
                return exolib.FloatTrackBarRanges
            case _:
                raise ValueError(f"Unknown item type: {type_name}")


class ParamNodeAsset:
    """param_node アセット"""

    def __init__(self, path: Path, asset_type: AssetType):
        """
        Parameters
        ----------
        path : Path
            アセットファイルのパス
        asset_type : AssetType
            system or user
        """
        self.path = path
        self.asset_type = asset_type
        self._load()

    def _load(self) -> None:
        """アセットファイルを読み込む"""
        with open(self.path, "r", encoding="utf-8") as file:
            data = yaml.safe_load(file)

            self.name = data.get("name", "")
            self.description = data.get("description", "")

            self.params: List[ParamNodeItemDefinition] = []
            for param in data.get("params", []):
                param_def = ParamNodeItemDefinition(param)
                self.params.append(param_def)

            self.trackbars: List[ParamNodeItemDefinition] = []
            for trackbar in data.get("trackbars", []):
                trackbar_def = ParamNodeItemDefinition(trackbar)
                self.trackbars.append(trackbar_def)

    def get_transformation_table(self) -> dict:
        """変換テーブルを取得する"""
        table = {
            ("_name", "_name"): exolib.String,
        }
        for param in self.trackbars + self.params:
            key = (param.exo_name, param.name)
            value = param.param_type
            table[key] = value
        return table

    def create_param_node(self, **params) -> DynamicObjectParamNode:
        """ObjectNodeに設定するObjectParamNodeを生成する"""
        transformation_table = self.get_transformation_table()
        default_params = {"_name": self.name} | {
            param.name: param.default for param in self.params + self.trackbars
        }
        return DynamicObjectParamNode(
            transformation_table=transformation_table,
            **default_params | params,
        )

    def get_mcp_resource(self) -> str:
        """MCP リソース用のテキストを取得する"""

        params = []
        for param in self.params:
            params.append(
                f"### {param.name}\n"
                + param.description
                + "\n\n"
                + f"param type: {param.param_type.__name__}\n"
                + f"default: {param.default}\n\n"
            )

        trackbars = []
        for trackbar in self.trackbars:
            trackbars.append(
                f"### {trackbar.name}\n"
                + trackbar.description
                + "\n\n"
                + f"param type: {trackbar.param_type.__name__}\n"
                + f"default: {trackbar.default}\n\n"
            )
        text = f"""
## name
{self.name}

## description
{self.description}

## available params
{"".join(params)}

## available trackbars
{"".join(trackbars)}
"""
        return text

    def __repr__(self) -> str:
        return f"ParamNodeAsset(path={self.path}, asset_type={self.asset_type})"

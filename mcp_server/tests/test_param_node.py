from pathlib import Path

import aviutl_mcp_server.param_node as param_node
import exolib
import pytest


class TestExoColor:
    def test_init(self):
        color = param_node.ExoColor([255, 0, 0])
        assert isinstance(color, exolib.Color)
        assert color.red == 255
        assert color.green == 0
        assert color.blue == 0

    def test_invalid_rgb_length(self):
        with pytest.raises(ValueError):
            param_node.ExoColor([255, 0])
        with pytest.raises(ValueError):
            param_node.ExoColor([255, 0, 0, 0])

    def test_copy_init(self):
        color = param_node.ExoColor([255, 0, 0])
        color_copy = param_node.ExoColor(color)
        assert isinstance(color_copy, exolib.Color)
        assert color_copy.red == 255
        assert color_copy.green == 0
        assert color_copy.blue == 0


class TestDynamicObjectParamNode:
    def test_init(self):
        transformation_table = {
            ("_name", "_name"): exolib.String,
        }
        params = {
            "_name": "テスト",
        }
        node = param_node.DynamicObjectParamNode(transformation_table, **params)
        assert isinstance(node, exolib.ObjectParamNode)
        assert type(node["_name"]) is exolib.String
        assert node["_name"] == "テスト"


class TestParamNodeItemDefinition:
    def test_init(self):
        param_data = {"name": "test_param", "type": "Int", "exo_name": "test_exo"}
        param = param_node.ParamNodeItemDefinition(param_data)
        assert param.name == "test_param"
        assert param.param_type == exolib.Int
        assert param.exo_name == "test_exo"

    def test_unknown_type(self):
        param_data = {
            "name": "test_param",
            "exo_name": "test_exo",
            "type": "UnknownType",
        }
        with pytest.raises(ValueError):
            param_node.ParamNodeItemDefinition(param_data)


class TestParamNodeAsset:
    def test_init(self):
        path = Path("assets/system/param_node/テキスト.yaml")
        asset_type = param_node.AssetType.SYSTEM
        asset = param_node.ParamNodeAsset(path, asset_type)
        assert asset.path == path
        assert asset.asset_type == asset_type
        assert asset.name == "テキスト"

    def test_load_all(self):
        """全てのアセットを読み込む"""
        system_path = Path("assets/system/param_node/")
        for p in system_path.glob("**/*.yaml"):
            try:
                _ = param_node.ParamNodeAsset(p, param_node.AssetType.SYSTEM)
            except Exception as e:
                pytest.fail(f"Failed to load {p}: {e}")

    def test_create_param_node(self):
        """テキストノードの生成"""
        path = Path("assets/system/param_node/テキスト.yaml")
        asset_type = param_node.AssetType.SYSTEM
        asset = param_node.ParamNodeAsset(path, asset_type)
        params = {
            "テキスト": "Hello, World!",
        }
        node = asset.create_param_node(**params)
        assert isinstance(node, param_node.DynamicObjectParamNode)
        assert node["_name"] == "テキスト"
        assert isinstance(node["テキスト"], exolib.Text)
        assert node["テキスト"] == "Hello, World!"
        assert isinstance(node["サイズ"], exolib.IntTrackBarRanges)
        assert node["サイズ"].values[0] == 34

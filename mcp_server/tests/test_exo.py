from pathlib import Path

import aviutl_mcp_server.exo as exo
import exolib
import pytest
from aviutl_mcp_server.assets import AssetManager
from exofile import TrackBarType


@pytest.fixture(scope="module")
def asset_manager():
    """Fixture for AssetManager"""
    return AssetManager(Path("./assets"))


@pytest.mark.parametrize(
    "type_name, expected_type",
    [
        ("None", TrackBarType.NONE),
        ("Linear", TrackBarType.LINEAR),
        ("Acceleration", TrackBarType.ACCELERATION),
        ("Curve", TrackBarType.CURVE),
        ("Teleportation", TrackBarType.TELEPORTATION),
        ("Ignore_midpoint", TrackBarType.IGNORE_MIDPOINT),
        ("Move_certain_amount", TrackBarType.MOVE_CERTAIN_AMOUNT),
        ("Random", TrackBarType.RANDOM),
        ("Repetition", TrackBarType.REPETITION),
        ("補間移動", "補間移動"),
        ("回転", "回転"),
    ],
)
def test_get_trackbar_type(type_name: str, expected_type: TrackBarType | str):
    assert exo.get_trackbar_type(type_name) == expected_type


class TestCreateParamNode:
    def test_create_param_node(self, asset_manager):
        """create_param_node のテスト"""
        node = {
            "name": "テキスト",
            "params": {
                "テキスト": "テスト",
                "文字色": [255, 0, 0],
            },
            "trackbars": {
                "サイズ": 100,
            },
        }
        param_node = exo.create_param_node(node, asset_manager)
        assert param_node is not None
        assert isinstance(param_node, exo.DynamicObjectParamNode)
        assert param_node["_name"] == "テキスト"
        assert param_node["テキスト"] == "テスト"
        assert param_node["文字色"].red == 255
        assert param_node["文字色"].green == 0
        assert param_node["文字色"].blue == 0
        assert isinstance(param_node["サイズ"], exolib.IntTrackBarRanges)


class TestCreateExo:
    def test_create_exo(self, asset_manager):
        """create_exo のテスト"""
        exo_data = {
            "overlay": True,
            "camera": False,
            "audio": False,
            "frame_range": [1, 31, 61, 91],
            "nodes": [
                {
                    "name": "テキスト",
                    "params": {
                        "テキスト": "テスト",
                        "文字色": [255, 0, 0],
                    },
                    "trackbars": {
                        "サイズ": 100,
                    },
                },
                {
                    "name": "標準描画",
                    "trackbars": {
                        "X": {
                            "points": [-200, 200, 200, -200],
                            "type": "Linear",
                            "accelerate": False,
                            "decelerate": False,
                            "parameter": None,
                        },
                        "Y": {
                            "points": [-200, -200, 200, 200],
                            "type": "Linear",
                            "accelerate": False,
                            "decelerate": False,
                            "parameter": None,
                        },
                    },
                },
            ],
        }
        exo_instance = exo.create_exo(exo_data, asset_manager)
        assert isinstance(exo_instance, exolib.EXO)
        assert exo_instance["length"] == 91

        object_nodes = list(exo_instance.iter_object())
        assert len(object_nodes) == 1

        object_node = object_nodes[0]
        assert object_node["overlay"] == 1
        assert object_node["camera"] == 0
        assert object_node["audio"] == 0
        assert len(object_node.midpoints) == 2
        assert object_node.midpoints[0] == 31
        assert object_node.midpoints[1] == 61

        param_nodes = list(object_node.iter_objparam())
        assert len(param_nodes) == 2
        assert param_nodes[0]["_name"] == "テキスト"
        assert param_nodes[1]["_name"] == "標準描画"

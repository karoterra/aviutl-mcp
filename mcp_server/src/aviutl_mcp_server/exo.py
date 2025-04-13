# exolibの検証用スクリプト


from exofile import TrackBarType
from exolib import (
    EXO,
    FloatTrackBarRanges,
    InsertionMode,
    IntTrackBarRanges,
    ObjectNode,
    PositionRange,
)

from aviutl_mcp_server.assets import AssetManager
from aviutl_mcp_server.param_node import DynamicObjectParamNode


def get_trackbar_type(type_name: str) -> TrackBarType | str:
    """トラックバーの種類を取得する"""
    match type_name:
        case "None":
            return TrackBarType.NONE
        case "Linear":
            return TrackBarType.LINEAR
        case "Acceleration":
            return TrackBarType.ACCELERATION
        case "Curve":
            return TrackBarType.CURVE
        case "Teleportation":
            return TrackBarType.TELEPORTATION
        case "Ignore_midpoint":
            return TrackBarType.IGNORE_MIDPOINT
        case "Move_certain_amount":
            return TrackBarType.MOVE_CERTAIN_AMOUNT
        case "Random":
            return TrackBarType.RANDOM
        case "Repetition":
            return TrackBarType.REPETITION
        case _:
            return type_name


def create_param_node(
    node: dict, asset_manager: AssetManager
) -> DynamicObjectParamNode | None:
    """ObjectParamNode を生成する"""
    param_node_asset = asset_manager.param_node_assets.get_asset(node["name"])
    if param_node_asset is None:
        return None

    param_node_params = {}
    params = node.get("params", {})
    for key, value in params.items():
        param_node_params[key] = value

    trackbars = node.get("trackbars", {})
    for key, value in trackbars.items():
        if type(value) is int or type(value) is float:
            param_node_params[key] = value
        elif type(value) is dict:
            points = value.get("points", [])
            trackbar_type = get_trackbar_type(value.get("type", "None"))
            accelerate = value.get("accelerate", False)
            decelerate = value.get("decelerate", False)
            parameter = value.get("parameter", None)

            trackbar_defs = list(
                filter(lambda x: x.name == key, param_node_asset.trackbars)
            )
            if len(trackbar_defs) == 0:
                continue
            trackbar_def = trackbar_defs[0]
            if trackbar_def.param_type == "IntTrackBar":
                trackbar = IntTrackBarRanges(
                    points,
                    trackbar_type,
                    accelerate=accelerate,
                    decelerate=decelerate,
                    parameter=parameter,
                )
            else:
                trackbar = FloatTrackBarRanges(
                    points,
                    trackbar_type,
                    accelerate=accelerate,
                    decelerate=decelerate,
                    parameter=parameter,
                )
            param_node_params[key] = trackbar

    return param_node_asset.create_param_node(**param_node_params)


def create_exo(object: dict, asset_manager: AssetManager) -> EXO:
    overlay = object.get("overlay", True)
    camera = object.get("camera", False)
    audio = object.get("audio", False)
    object_node = ObjectNode(overlay=overlay, camera=camera, audio=audio)

    frame_range = object.get("frame_range", [1, 11])
    frame_range = sorted(set(map(int, frame_range)))
    position_range = PositionRange(frame_range[0], frame_range[-1])

    nodes = object.get("nodes", [])
    for node in nodes:
        param_node = create_param_node(node, asset_manager)
        if param_node:
            object_node.add_objparam(param_node)

    if len(frame_range) > 2:
        for p in frame_range[1:-1]:
            object_node.add_midpoint(p)

    exo = EXO()
    exo.insert_object(1, position_range, object_node, InsertionMode.SHIFT_RIGHT)
    exo.fit_length_to_last_object()
    return exo

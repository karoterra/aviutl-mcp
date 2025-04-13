from pathlib import Path

from aviutl_mcp_server.param_node import ParamNodeAsset
from aviutl_mcp_server.types import AssetType


class ParamNodeAssetManager:
    def __init__(self, system_dir: Path, user_dir: Path):
        """
        Parameters
        ----------
        path : Path
            assets ディレクトリのパス
        """
        self.assets: list[ParamNodeAsset] = []
        self._load_dir(system_dir, AssetType.SYSTEM)
        self._load_dir(user_dir, AssetType.USER)

    def _load_dir(self, path: Path, asset_type: AssetType) -> None:
        """アセットディレクトリを読み込む"""
        for file in path.glob("**/*.yaml"):
            self.assets.append(ParamNodeAsset(file, asset_type))

    def get_asset(self, name: str) -> ParamNodeAsset | None:
        """アセットを取得する"""
        for asset in self.assets:
            if asset.name == name:
                return asset
        return None


class AssetManager:
    def __init__(self, asset_path: Path):
        self.asset_path = asset_path
        self.param_node_assets = ParamNodeAssetManager(
            asset_path / AssetType.SYSTEM / "param_node",
            asset_path / AssetType.USER / "param_node",
        )

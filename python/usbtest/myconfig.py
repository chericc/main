import json
from pathlib import Path
from typing import Dict, Any
import logging

class MyConfig:
    hub_port_count: int = 10
    wait_port_seconds: int = 15
    camera_version_file_path: str = 'sf_version.txt'
    camera_update_dst_file_path: str = 'FIRMWARE/update.zip'
    camera_update_file_from_path: str = 'd:/tmp/130.20250307.zip'
    screen_version_file_path: str = 'version.txt'
    screen_update_dst_file_path: str = 'update/update.bin'
    screen_update_file_from_path: str = 'd:/tmp/screen.20250307.bin'
    com_port: str = 'COM19'
    log_file: str = 'testapp.log'
    log_level_file: str = 'DEBUG' # DEBUG INFO WARNING ERROR
    log_level_console: str = 'INFO'
    update_interval_sec: int = 2

    _config_file: str = 'config.json'

    def __init__(self, config_file: str = 'config.json'):
        if config_file:
            self._config_file = config_file
        self.load_config()

    def load_config(self) -> None:
        """从JSON文件加载配置"""
        if Path(self._config_file).exists():
            with open(self._config_file, 'r', encoding='utf-8') as f:
                config_data: Dict[str, Any] = json.load(f)
                for key, value in config_data.items():
                    if hasattr(self, key):
                        setattr(self, key, value)
        logging.info('save configs')
        self.save_config()

    def save_config(self) -> None:
        """将当前配置保存到JSON文件"""
        config_data = {key: getattr(self, key)
                       for key in dir(self)
                       if not key.startswith('_') and not callable(getattr(self, key))}
        with open(self._config_file, 'w', encoding='utf-8') as f:
            json.dump(config_data, f, indent=4)

    def map_log_level(self, log_level: str) -> int:
        if log_level == 'DEBUG':
            return logging.DEBUG
        elif log_level == 'INFO':
            return logging.INFO
        elif log_level == 'WARNING':
            return logging.WARNING
        elif log_level == 'ERROR':
            return logging.ERROR
        else:
            return logging.INFO

g_config = MyConfig()
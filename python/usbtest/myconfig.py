class MyConfig:
    hub_port_count: int = 10
    wait_port_seconds: int = 15
    camera_version_file_path: str = 'sf_version.txt'
    camera_update_dst_file_path: str = 'FIRMWARE/update.zip'
    camera_update_file_from_path: str = 'd:/tmp/130.20250307.zip'
    screen_version_file_path: str = 'version.txt'
    screen_update_dst_file_path: str = 'update/update.bin'
    screen_update_file_from_path: str = 'd:/tmp/screen.20250307.bin'
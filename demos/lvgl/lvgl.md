# lvgl

## setup - lv_port_linux

```bash
https://github.com/lvgl/lv_port_linux

sudo apt install libsdl2-dev libsdl2-image-dev
sudo apt install libevdev-dev
sudo apt install libdecor-0-dev
sudo apt install libwayland-dev

# edit lv_conf.h
# LV_USE_SDL --> 1
# LV_USE_LINUX_FBDEV --> 0
```

## set - imdepedent lvgl demo

### cmds

```bash
export OPENSRC_LIB_PATH=~/opensrc
export CMAKE_INCLUDE_PATH=$OPENSRC_LIB_PATH/lvgl/lvgl/build/output/include
export CMAKE_LIBRARY_PATH=$OPENSRC_LIB_PATH/lvgl/lvgl/build/output/lib
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

```bash
# clone lvgl
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$(pwd)/output
```


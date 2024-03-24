# env-setup


## environment

```bash
# glfw
sudo apt-get install libglfw3-dev -y

sudo apt-get install libglu1-mesa-dev -y

# glew
sudo apt-get install libxmu-dev libxi-dev libgl-dev -y

https://sourceforge.net/projects/glew/
make & sudo make install

# 

```

## build

```bash

g++ src.cpp -lglfw -lGLEW -lGL 

```
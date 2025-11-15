# 终端开发环境

## terminal

wezterm

```lua
-- Pull in the wezterm API
local wezterm = require 'wezterm'

-- This will hold the configuration.
local config = wezterm.config_builder()

-- This is where you actually apply your config choices.

-- For example, changing the initial geometry for new windows:
config.initial_cols = 120
config.initial_rows = 28

-- or, changing the font size and color scheme.
config.font_size = 13
config.color_scheme = 'AdventureTime'
config.font = wezterm.font '0xProto Nerd Font Mono'

-- Finally, return the configuration to wezterm:
return config
```

## neovim

```bash
https://github.com/neovim/neovim/releases/download/v0.11.5/nvim-linux-x86_64.tar.gz

git clone https://github.com/LazyVim/starter ~/.config/nvim
rm -rf ~/.config/nvim/.git
```

## lazyvim

https://www.lazyvim.org/
sudo apt install python3-venv -y

## 

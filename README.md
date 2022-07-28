# yaltl

yaltl is a terminal launcher

yaltl - Yet Another Linux Terminal Launcher

![yaltl demo](./images/yaltl.gif)

## Dependencies

- [clang](http://llvm.org/)
  - C++20
- [ftxui](https://github.com/ArthurSonzogni/FTXUI) - For TUI
- [giomm](https://developer.gnome.org/glibmm/stable/) - For drun
- [gtkmm](https://www.gtkmm.org/en/) - For recent documents
- [i3ipcpp](https://github.com/drmgc/i3ipcpp) - For i3/sway window switching
- [pcre2](https://www.pcre.org/current/doc/html/index.html) - regex (will fallback to C++11 regex implementation if not found)
- [mtl](https://github.com/scaryrawr/mtl) - For vanity

## Building

```sh
cd yaltl
export CC=clang
export CXX=clang++
mkdir build && cd build
cmake ..
make
sudo make install
```

## Modes

- dmenu - Only accessibly by using `-d` or `--dmenu`
  - Reads from stdin and outputs selection to stdout
  - Will not run with any other modes since there may be unexpected behavior
- drun - Run from installed desktop applications
- recent - Lists recently used documents to open
- run - Run binary from PATH
- i3wm - Window switcher for i3wm/sway
- Script - Run a script
  - Results will be passed back to the script
  - Continued output to stdout will cause yaltl to continue to display the new results

## Keyboard shortcuts

- Esc - Cancel out of yaltl
- Tab - Next Mode
- Shift+Tab - Previous Mode

## Configuring i3/Sway

Example uses [alacritty](https://github.com/alacritty/alacritty)

### [Sway](https://github.com/swaywm/sway)

Set up your menu command:

```shell
for_window [app_id="yaltl"] floating enable, border pixel 2, sticky enable
set $menu exec alacritty --class yaltl -d 80 10 -e sh -c 'yaltl -m drun,run,i3wm'
```

### [i3](https://i3wm.org/)

i3 appears to have parse errors when trying to launch yaltl with args, so we use a launch script.

Create a launch script `~/.config/i3/scripts/launch_yaltl`:

```sh
#!/usr/bin/env sh
yaltl -m drun,run,i3wm
```

Set up your menu command:

```shell
for_window [instance="yaltl"] floating enable, border pixel 2, sticky enable
set $menu exec alacritty --class yaltl -d 80 10 -e sh -c '~/.config/i3/scripts/launch_yaltl'
```

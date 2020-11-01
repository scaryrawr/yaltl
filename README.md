# tofi

tofi is a terminal launcher

Why tofi? A play on [toffee](https://en.wikipedia.org/wiki/Toffee), and because there's:

- [rofi](https://github.com/davatorium/rofi)
- [wofi](https://hg.sr.ht/~scoopta/wofi)

And it's based on using the terminal for UI.

![tofi demo](./images/tofi.gif)

## Dependencies

- [clang](http://llvm.org/)
- [ftxui](https://github.com/ArthurSonzogni/FTXUI)
- [giomm](https://developer.gnome.org/glibmm/stable/)
- [i3ipcpp](https://github.com/drmgc/i3ipcpp)
- [mtl](https://github.com/scaryrawr/mtl)

## Building

```sh
cd tofi
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
- run - Run binary from PATH
- i3wm - Window switcher for i3wm/sway
- Script - Run a script
  - Results will be passed back to the script
  - Continued output to stdout will cause tofi to continue to display the new results

## Keyboard shortcuts

- Ctrl+]/Esc - Cancel out of tofi
- Tab - Next Mode
- Shift+Tab - Previous Mode

## Configuring i3/Sway

Example uses [alacritty](https://github.com/alacritty/alacritty)

### [Sway](https://github.com/swaywm/sway)

Set up your menu command:

```shell
for_window [app_id="tofi"] floating enable, border pixel 2, sticky enable
set $menu exec alacritty --class tofi -d 80 10 -e sh -c 'tofi -m drun,run,i3wm'
```

### [i3](https://i3wm.org/)

i3 appears to have parse errors when trying to launch tofi with args, so we use a launch script.

Create a launch script `~/.config/i3/scripts/launch_tofi`:
```sh
#!/usr/bin/env sh
tofi -m drun,run,i3wm
```

Set up your menu command:

```shell
for_window [instance="tofi"] floating enable, border pixel 2, sticky enable
set $menu exec alacritty --class tofi -d 80 10 -e sh -c '~/.config/i3/scripts/launch_tofi'
```

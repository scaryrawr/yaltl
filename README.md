# tofi

tofi is a terminal launcher

Why tofi? A play on [toffee](https://en.wikipedia.org/wiki/Toffee), and because there's:

- [rofi](https://github.com/davatorium/rofi)
- [wofi](https://hg.sr.ht/~scoopta/wofi)

And it's based on using the terminal for UI.

## Dependencies

- [ftxui](https://github.com/ArthurSonzogni/FTXUI)
- [giomm](https://developer.gnome.org/glibmm/stable/)
- [i3ipcpp](https://github.com/drmgc/i3ipcpp)

## Building

```sh
cd tofi
mkdir build && cd build
cmake ..
make
sudo make install
```

## Modes

- drun - Run from installed desktop applications
- run - Run binary from PATH
- i3wm - Window switcher for i3wm/sway

## Keyboard shortcuts

- Ctrl+] - Cancel out of tofi
- Tab - Next Mode
- Shift+Tab - Previous Mode

## Configuring i3/Sway

Example uses [alacritty](https://github.com/alacritty/alacritty)

### [Sway](https://github.com/swaywm/sway)

Set up your menu command:

```shell
for_window [app_id="tofi"] floating enable, border pixel 2, sticky enable
set $menu exec alacritty --class tofi -d 80 10 -e sh -c 'tofi'
```

### [i3](https://i3wm.org/)

Set up your menu command:

```shell
for_window [instance="tofi"] floating enable, border pixel 2, sticky enable
set $menu exec alacritty --class tofi -d 80 10 -e sh -c 'tofi'
```

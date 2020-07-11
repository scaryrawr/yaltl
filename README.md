# tofi

tofi is a terminal launcher for i3/sway using [fzf](https://github.com/junegunn/fzf)

## Dependencies

- [i3ipc](https://github.com/altdesktop/i3ipc-python)
- [fzf](https://github.com/junegunn/fzf)
- [PyGObject](https://pygobject.readthedocs.io/en/latest/)

## Scripts

- tofi - launcher script
- tofib - tofi backend

## Modes

- ctrl+a - Application Launcher (default)
- ctrl+w - Active Window Select

## Configuring i3/Sway

Example uses [alacritty](https://github.com/alacritty/alacritty)

### [Sway](https://github.com/swaywm/sway)

Set up your menu command:

```shell
for_window [app_id="tofi"] floating enable, border pixel 2, sticky enable

# Update for the path for where you clone
set $menu exec alacritty --class tofi -d 80 10 -e sh -c '${HOME}/GitHub/tofi/tofi'
```

### [i3](https://i3wm.org/)

Set up your menu command:

```shell
for_window [instance="tofi"] floating enable, border pixel 2, sticky enable

# Update for the path for where you clone
set $menu exec alacritty --class tofi -d 80 10 -e sh -c '${HOME}/GitHub/tofi/tofi'
```

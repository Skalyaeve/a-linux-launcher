<img align="center" src="https://github.com/Skalyaeve/images-2/blob/main/screenshot/launcher.gif?raw=true"></img>

The program reads `.desktop` files from `path` (see config), using the following entries:

- `Type` - Entry type: `Application` | `Link`
- `Name` - Entry identifier
- `Exec` | `URL` - Command to execute | URL to open
- `Terminal` - Whether to run in terminal

Entries can be grouped by categories using file system tree from `path`.
For exemple, having a valid entry file in `path/folder/app.desktop` creates a category `folder` and makes `app.desktop` an entry of it

## Config

- `x`: Box position on the x-axis
- `y`: Box position on the y-axis
- `x-padding`: Space between box border and text on the x-axis
- `y-padding`: Space between box border and text on the y-axis
- `border-size`: Box border size
- `line-margin`: Space between lines
- `window-margin`: Space between boxes
- `max-lines`: Maximum number of lines for a box (aka max-height)
- `max-len`: Maximum number of characters for a line (aka max-width)
- `font-size`: This is in fact used to set line height
- `font`: This is the actual font size & family to use (see `xlsfonts`)
- `bg-color`: Box background color
- `fg-color`: Box text color
- `focus-bg-color`: Selected line background color
- `focus-fg-color`: Selected line text color
- `border-color`: Box border color
- `path`: Path to read `.desktop` files from
- `terminal`: Terminal command to use
- `shell`: Shell command to use
- `browser`: Browser command to use
- `search-engine`: Search engine command to use, this is concatenated with `browser`

> Any font can be converted to a X11 font, ask Google

## Install

```bash
apt update -y
apt install -y git
apt install -y make
apt install -y gcc
apt install -y libx11-dev
mkdir -p ~/.local/src
mkdir -p ~/.local/bin
mkdir -p ~/.config
```

```bash
name=launcher
cd ~/.local/src
git clone https://github.com/Skalyaeve/a-linux-launcher.git $name
cd $name
make && make install
```

```bash
export PATH=$HOME/.local/bin:$PATH
# or ln -s $HOME/.local/bin/launcher /usr/local/bin/launcher
launcher ~/.config/launcher/config
```

> [!WARNING]
> If the process is killed (`SIGKILL`) or closed with something else than `ESC`, `SIGINT`, `SIGTERM` or `SIGQUIT`, you will have to remove the lock file located at `/tmp/launcher.ft.lock` before you can run the program again

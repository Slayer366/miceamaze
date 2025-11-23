MiceAmaze
=========

MiceAmaze is a free video game that features a maze with mice and snakes.

Forked from: https://github.com/rchampeimont/miceamaze

Official website: http://www.miceamaze.org/

Has been converted to SDL2

Text/font size has been increased for visibility on low-res devices.

MSAA has been enabled to make text more visible on low-res.

Now uses current directory for data and config for portability.

To build on Linux:

```bash
sudo apt install libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev libfreetype6-dev
make -j$(nproc)
```

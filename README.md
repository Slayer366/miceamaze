MiceAmaze
=========

MiceAmaze is a free video game that features a maze with mice and snakes.

Forked from: https://github.com/rchampeimont/miceamaze

Official website: http://www.miceamaze.org/

Has been converted to SDL2

Text/font size has been increased for visibility on low-res devices.

MSAA x1 has also been enabled to make text more visible on low-res.

Now uses current directory for data and config for portability.

To build on Linux:

```bash
sudo apt install libsdl2-dev libfreetype6-dev libglc-dev libsoil-dev 
make -j$(nproc)
```

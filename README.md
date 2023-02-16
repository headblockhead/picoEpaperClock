# picoEpaperClock
An alarm clock written in C and made for the pico + pico-waveshare epaper display.

## Tasks

### Build
Builds the project.
```bash
mkdir -p build
cd build
cmake ..
make
cd ..
```
### Deploy
Requires: Build
Builds, then uploads the project to the pico.
```bash
cd build
sudo picotool load ./clock.uf2 -F
sudo picotool reboot -F
```

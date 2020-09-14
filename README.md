## VideoSwitch
Switch between videos and interactive word cloud.

### Requirements
* wordcloud from https://github.com/amueller/word_cloud
  (install with `pip3 install wordcloud` or using your package manager)
* Qt5, including multimedia widgets
* inkscape, for converting .svg image to .png image with correct resolution.
  This could also be done using, e.g., ImageMagick (`convert`), but that might lead to problems with the background color.

In Arch:
Install the packages inkscape, qt5-multimedia, gst-libav, gst-plugins-good, and from the AUR wordcloud.

In ubuntu (note that this is not a clean way of installing software!):
```sh
# Install required packages
sudo apt install inkscape python3-pip g++ qt5-qmake qt5-default qtmultimedia5-dev libqt5multimedia5-plugins
# Install wordcloud using pip
pip3 install wordcloud
export PATH="$PATH:/home/$USER/.local/bin" # if you use pip as normal user
```

### Build
```sh
qmake && make
```

### Usage
Before starting `switchvideo`, some preparation is needed. Everything needs to be adapted to the screen resolution.
For a resolution 1280x720, use the following commands.
```sh
WIDTH=1280 HEIGHT=720 bash prepare_and_run.sh
```
The following files are required (default paths are hard-coded in `mainwindow.h`):
* `gen_wordcloud.py` for generating the word cloud image `/tmp/wordcloud.png`
* `/tmp/wordlist.txt` is the source for the word cloud. `switchvideo` appends words to that file.
* `playlist.json` contains a mapping of tites to video paths. The titles will be shown as push buttons in the GUI of videoswitch.

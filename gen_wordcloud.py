#!/bin/env python3
'''
Generate a word cloud of /tmp/wordlist.txt in /tmp/wordcloud.png.
'''

from PIL import Image
import numpy as np
from wordcloud import WordCloud
#from wordcloud import ImageColorGenerator
from matplotlib.colors import LinearSegmentedColormap


# Define a colormap of different green shades
cm_r = lambda x: 0.6 * (x < 0.75) * (0.75 - x)
cm_g = lambda x: 1. - 0.4 * (x > 0.5) * (x - 0.5)**2
cm_b = lambda x: 0.5 * (x > 0.5) * (x - 0.5)
cm = LinearSegmentedColormap('gauss', {'red':cm_r, 'green':cm_g, 'blue':cm_b}, 256, gamma=1)


def exportWordcloud(
            textfile : "input"  = "/tmp/wordlist.txt",
            maskfile : "mask"   = "mask.png",
            outfile  : "output" = "/tmp/wordcloud.png",
        ):
    text = open(textfile, encoding="utf-8").read()
    color = np.array(Image.open(maskfile))
    mask = 255 * (color[...,0] * color[...,1] * color[...,2] == 0)
    #image_colors = ImageColorGenerator(color)
    wc = WordCloud(
            max_font_size = 250,
            #font_path = "/usr/share/fonts/titillium/Titillium-Regular.otf",
            mask = mask,
            random_state = 42,
            #regexp = "[A-ZÄÖÜ][a-zäöüßA-ZÄÖÜ0-9-*]+", # for German texts
            min_word_length = 3,
            collocations = False,
            relative_scaling = 0.2,
            #color_func = image_colors,
            colormap = cm,
        )
    wc.generate(text)
    wc.to_file(outfile)


def print_help():
    print('''Usage: python gen_wordcloud.py [key=value ...]\nAvailable options:
    input  : text file containing words to generate word cloud
    mask   : image file to generate mask for word cloud
    output : file name of output image''')


if __name__ == '__main__':
    from sys import argv, stderr
    mapback = {key:value for (value,key) in exportWordcloud.__annotations__.items()}
    kwargs = {}
    for arg in argv[1:]:
        try:
            key, value = arg.split("=")
            try:
                kwargs[mapback[key]] = value
            except KeyError:
                if key in exportWordcloud.__annotations__:
                    kwargs[key] = value
                else:
                    print("Unknown argument key %s."%key, file=stderr)
                    print_help()
                    exit(1)
        except:
            print("Argument %s not understood: Must be of the form key=value."%arg, file=stderr)
            print_help()
            exit(1)
    exportWordcloud(**kwargs)

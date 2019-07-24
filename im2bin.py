#!/usr/bin/python3

import os, sys
from PIL import Image

WIDTH  = 256
HEIGHT = 128
BPP    = 2

rgba8_to_rgba4 = lambda r, g, b, a: \
    ((r >> 4) & 0xf) + (((g >> 4) & 0xf) << 4) + (((b >> 4) & 0xf) << 8) + (((a >> 4) & 0xf) << 12)

def resize(im):
    return im.resize((WIDTH, HEIGHT), Image.LANCZOS)

def dump(im, outf=os.path.join(os.path.dirname(__file__), "data", "im.bin")):
    with open(outf, "wb") as fp:
        for p in list(im.getdata()):
            fp.write(rgba8_to_rgba4(*p).to_bytes(BPP, byteorder="little"))
    assert(os.path.getsize(outf) == WIDTH * HEIGHT * BPP)

def main(argc, argv):
    if (argc != 2):
        print(f"Usage: {argv[0]} im")
        return 1
    dump(resize(Image.open(argv[1])))
    return 0

if __name__ == "__main__":
    sys.exit(main(len(sys.argv), sys.argv))

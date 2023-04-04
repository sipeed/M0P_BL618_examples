import sys
import cv2 as cv
import numpy as np
import os

from socket import *


def up_img(img, ip_addr, port):
    addr = (ip_addr, port)
    sock_client = socket(AF_INET, SOCK_DGRAM)
    sock_client.sendto(np.array([0x11, 0x00], 'uint8'), addr)
    for im in img.reshape((-1, 320*2*16)):
        sock_client.sendto(im, addr)


def open_img(path):
    if not os.path.exists(path):
        exit("error: no img:" + path)
    raw_image_file = path
    rgb565 = cv.cvtColor(cv.resize(cv.imread(raw_image_file), dsize=(
        320, 320), interpolation=cv.INTER_CUBIC), cv.COLOR_BGR2BGR565)  # [:,:,::-1]
    print("pushing " + path + " shape: ", rgb565.shape)
    return rgb565


IP_ADDR = "192.168.3.1"
PORT = 5001


def main():
    argv = sys.argv
    argc = len(argv)
    if argc != 2:
        exit(argv[0]+" img")
    img = open_img(argv[1])
    up_img(img, IP_ADDR, PORT)


if __name__ == "__main__":
    main()

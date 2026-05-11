# -*- coding: utf-8 -*-
# base64 test
# 把二进制数据，按 6bit 一组，映射成可打印字符

import base64

# encode
def encode(b64: str):
    b64 = base64.b64encode(b64.encode("utf-8")).decode("utf-8")
    print(base64.b64encode(b64.encode("utf-8")))
    return b64


# decode
def decode(b64: str):
    origin = base64.b64decode(b64).decode("utf-8")
    return origin


if __name__ == "__main__":

    print("hello world")

    s = "orianna_a24@foxmail.com"
    b64 = encode(s)
    print(b64)

    origin = decode(b64)
    print(origin)

# "b3JpYW5uYV9hMjRAZm94bWFpbC5jb20="

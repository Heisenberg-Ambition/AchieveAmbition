#
#
#

g++ test.cpp -o combine \
  $(pkg-config --cflags --libs libavformat libavcodec libavfilter libavutil libswscale)

# v4l

## Howto convert to suitable input
ffmpeg -i <input_file> -pix_fmt yuv420p -c:v rawvideo -an -s 560x320 -y <outputfile.yuv>

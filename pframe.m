width = 560;
height = 320;

fmsize = width*height;

fp = fopen('output.yuv');

frame = fread(fp, [width, height], 'uint8')';
fseek(fp, width*height/2, 'cof');
frame2 = fread(fp, [width, height], 'uint8')';

imshow(frame2-frame,[0,255]);


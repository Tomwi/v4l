width = 560;
height = 320;

fmsize = width*height;

fp = fopen('output.yuv');

frame = fread(fp, [width, height], 'uint8')';
fclose(fp);

figure
imshow(frame, [0,255]);

% Apply DWHT on all 8x8 blocks. Discard half of coefficients.
% Generate WHT matrix (reorder sequences etc.)
ident = eye(8,8);
H = fwht(ident)*8; 
Hinv = ifwht(ident)/8;

result = zeros(size(frame));
decompressed = zeros(size(frame));
for y=1:height/8
    for x=1:width/8
        % select block. Transform. Throw away coefficients.
        block = frame((y-1)*8+1:y*8,(x-1)*8 + 1:x*8);
        transf = H*block*H';
        transf = fix(transf); % effectively decimation by 64
        transf = fix(transf/64);
%        transf = rot90(tril(rot90(transf)),3);
 %       transf = rot90(tril(rot90(transf(1:6,1:6))),3);
        
       % transf(8,8)=0;
     %   transf = transf;
        result((y-1)*8+1:y*8, (x-1)*8 + 1:x*8) = transf;
        decompressed((y-1)*8+1:y*8, (x-1)*8 + 1:x*8) = Hinv'*transf*Hinv; 
    end
end

figure
imshow(decompressed*64, [0,255]);

fp = fopen('compressed', 'wb');
fwrite(fp, result);
fclose(fp);


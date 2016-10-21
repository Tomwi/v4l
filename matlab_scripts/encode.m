WIDTH=1280;
HEIGHT = 720;
FRAME_SIZE = WIDTH*HEIGHT * 1.5;

FILE = '/home/tomwi/Videos/noaudio.nv12';

fp = fopen(FILE);
% First frame is black.
fseek(fp, 300*FRAME_SIZE, 'bof');
% Read first frame, we'll intra code it
frame1 = fread(fp, [WIDTH, HEIGHT], 'uint8')';

% Skip chroma plane
fseek(fp, FRAME_SIZE/3, 'cof');
frame2 = fread(fp, [WIDTH, HEIGHT], 'uint8')';
fclose(fp);

% Construct a Walsh-hadamard matrix
ident = eye(8,8);
H = fwht(ident)*8; 
Hinv = ifwht(ident)/8;

result = zeros(size(frame1));
decompressed = zeros(size(frame1));

% First frame
for y=1:HEIGHT/8
    for x=1:WIDTH/8
           % select block. Transform. Throw away coefficients.
            block = frame1((y-1)*8+1:y*8,(x-1)*8 + 1:x*8);
            transf = H*block*H';
            transf = floor(floor(transf)/4); % Emulate integer calculations
	    decompressed((y-1)*8+1:y*8, (x-1)*8 + 1:x*8) = Hinv'*transf*Hinv*4; 
    end
end

% Second frame-pframe
diff = frame2-frame1;

for y=1:HEIGHT/8
    for x=1:WIDTH/8
	    % compare variance 
        blockc = frame2((y-1)*8+1:y*8,(x-1)*8 + 1:x*8);
	    blocke = diff((y-1)*8+1:y*8,(x-1)*8 + 1:x*8);
	    blockp = frame1((y-1)*8+1:y*8,(x-1)*8 + 1:x*8);

	    varc = var(reshape(blockc, 1, 64),1);
	    vard = var(reshape(blocke, 1, 64),1);

	    if(varc <= vard)
		transf = H*blockc*H';						    
       		transf = floor(floor(transf)/4);		
	    	decompressed2((y-1)*8+1:y*8, (x-1)*8 + 1:x*8) = Hinv'*transf*Hinv*4; 
             result((y-1)*8+1:y*8, (x-1)*8 + 1:x*8) = transf;
	    else
		transf = H*blocke*H';
       		transf = floor(floor(transf)/8);		 
            result((y-1)*8+1:y*8, (x-1)*8 + 1:x*8) = transf;
	    	decompressed2((y-1)*8+1:y*8, (x-1)*8 + 1:x*8) = Hinv'*transf*Hinv*8+blockp; 
            %mijn2((y-1)*8+1:y*8, (x-1)*8 + 1:x*8) = mijn2((y-1)*8+1:y*8, (x-1)*8 + 1:x*8) +blockp; 
       

	    end
    end
end
imshow(decompressed2, [0,255]);


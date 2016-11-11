close all
file = 'park';
statistics_dir = 'park_acceptable/';
output_suffix = 'park_acceptable';

width = 1280;
height = 720;
frames = 500;

fmsize = width*height;

fp2 = fopen(strcat(strcat('/run/media/tomwi/data/', statistics_dir),'/rawoutput'));
string = strcat(strcat('/run/media/tomwi/data/', file), '.yuv')
fp = fopen(string);

kwaliteit = [];
% for i=1:frames
%     frame = fread(fp, [width, height], 'uint8');
%     frame2 = fread(fp2, [width, height], 'uint8');
%     fseek(fp, width*height*0.5, 'cof');
%     fseek(fp2, width*height*0.5, 'cof');
%     kwaliteit(i) = psnr(uint8(frame), uint8(frame2));
% end

compressies = csvread(strcat(strcat('/run/media/tomwi/data/', statistics_dir), 'cratio.txt'));
tijden = csvread(strcat(strcat('/run/media/tomwi/data/', statistics_dir), 'times.txt'));

maxctijd = max(tijden(:,1))/1000;
maxdtijd = max(tijden(:,2))/1000;
minctijd = min(tijden(:,1))/1000;
mindtijd = min(tijden(:,2))/1000;
meanctijd = mean(tijden(:,1))/1000;
meandtijd = mean(tijden(:,2))/1000;

fpt = fopen(strcat(output_suffix, '_times_encoder.tex'), 'wt');
fprintf(fpt, '\\begin{tabular}{c|c|c|c}\n\\textbf{min (ms)} & \\textbf{mean (ms)} & \\textbf{max (ms)} & \\textbf{total(ms)}\\\\\\hline\n');
fprintf(fpt, '%.2f & %.2f & %.2f & %.2f\\\\\n', minctijd, meanctijd, maxctijd, sum(tijden(:,1)/1000));
fprintf(fpt, '\\end{tabular}\n\n\\caption{Encoder times}\n');
fclose(fpt);

fpt = fopen(strcat(output_suffix, '_times_decoder.tex'), 'wt');
fprintf(fpt, '\\begin{tabular}{c|c|c|c}\n\\textbf{min (ms)} & \\textbf{mean (ms)} & \\textbf{max (ms)} & \\textbf{total(ms)} \\\\\\hline\n');
fprintf(fpt, '%.2f & %.2f & %.2f & %.2f\\\\\n', mindtijd, meandtijd, maxdtijd, sum(tijden(:,2)/1000));
fprintf(fpt, '\\end{tabular}\n\\caption{Decoder times}\n');
fclose(fpt);
% figure
% plot(tijden(:,1), 'r-');
% hold on
% plot(tijden(:,2), 'g-.');
% print(strcat(output_suffix,'_times'),'-dtikz');
% figure
% plot(compressies(:,1), 'r-');
% hold on
% plot(compressies(:,2), 'b-');
% xlabel('Frame number', 'FontSize',16) % x-axis label
% ylabel('Compressio ratio (\%)','FontSize',16) % y-axis label
% h_legend = legend('Y','UV')
% set(h_legend,'FontSize',16);
% set(gca,'fontsize',16)
% print(strcat(output_suffix,'_ratios' ),'-dtikz');
%
% figure
% plot(kwaliteit, '-');
% xlabel('Frame number', 'FontSize',16) % x-axis label
% ylabel('PSNR (dB)','FontSize',16) % y-axis label
% set(gca,'fontsize',16)
% print(strcat(output_suffix, '_psnr' ),'-dtikz');

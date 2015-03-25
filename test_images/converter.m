function converter(inDir, outDir)
%CONVERTER Summary of this function goes here
%   Detailed explanation goes here

images = dir([inDir, filesep, '*.png']);
for i = 1:length(images)
    img_name = images(i).name;
    
    fprintf('Processing file %s...\n', img_name);
    img_file = [inDir, filesep, img_name];
    
    img = imread(img_file);
    [r, c, ~] = size(img);
    out = -1*ones(r, c);
    
    out(img(:, :, 1) < 50) = 1;
    [name, ~] = strtok(img_name, '.');
    dlmwrite([outDir, filesep, name, '.dlm'], out, ' ');
end
end


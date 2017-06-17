clear all;
close all;
 
% load image sequence
fileFolder = 'All';
dirOutput = dir(fullfile(fileFolder, '/*.png'));
fileNames = {dirOutput.name};
numFrames = numel(fileNames)
I = imread(fileNames{1});
 
sequence = zeros([size(I(:, :, 1)) numFrames], 'uint16');
for p = 1:numFrames
    I = imread(fileNames{p});
    red = I(:, :, 1);
    green = I(:, :, 2);
    blue = I(:, :, 3);
    %slice = zeros([size(I(:, :, 1))], 'uint16');
    slice = uint16(red) * 65536 +  uint16(green) * 256 +  uint16(blue) ;
    % or if image has one channel
    % slice = I;
    sequence(:, :,  p) = slice;
end 

setIDs = unique(sequence);

ID = 745 % substitute with object of interest ID

% generate a mask with the ID we want
mask = (sequence == ID);
mask = mask * 255;
   
% 3D gaussian blur
sigma = 8;
volSmooth = imgaussfilt3(mask, sigma);
    
% or if the object is too small ex. glycogen granules 
% and needs more blurring effect
% sigma = 3;
% volSmooth = mask;
% for i = 1:20
%     volSmooth = imgaussfilt3(volSmooth, sigma, 'FilterSize', 21);
%     volSmooth = volSmooth + mask;
%     % clamp
%     volSmooth = max(min(volSmooth,255),0);
% end


dims = size(volSmooth(1, 1, :))
slices = dims(3);
folderName = 'binary_image_stack'
mkdir(folderName);
volSmooth = uint8(volSmooth);

% iterate over the images and write them in a folder as pngs
for j = 1:slices
    imageName = strcat(folderName,'/', num2str(j-1), '.png')
    imwrite(volSmooth(:, :, j), imageName);
end
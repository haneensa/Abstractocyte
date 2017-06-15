clear all;
close all;
 
% load image sequence
fileFolder = 'All';
dirOutput = dir(fullfile(fileFolder, '/*.png'));
fileNames = {dirOutput.name};
numFrames = numel(fileNames)
I = imread(fileNames{1});

% get the IDs for the first slice by combining the three channels RGB
red = I(:, :, 1);
green = I(:, :, 2);
blue = I(:, :, 3);
slice = zeros([size(I(:, :, 1))], 'uint16');
slice = uint16(red) * 265536 + uint16(green) * 256 + uint16(blue); 

sequence = zeros([size(I(:, :, 1)) numFrames], 'uint16');
sequence(:, :,  1) = slice;

% get the IDs for all the slices into 'sequence' matrix
for p = 2:numFrames
    I = imread(fileNames{p});
    red = I(:, :, 1);
    green = I(:, :, 2);
    blue = I(:, :, 3);
    slice = zeros([size(I(:, :, 1))], 'uint16');
    slice = uint16(red) * 265536 + uint16(green) * 256 + uint16(blue); 
    sequence(:, :,  p) = slice;
end 

% get the set of unique IDs in the segmentation stack
setIDs = unique(sequence);

% for loop over setIDs, mask the data and leave one ID at a time
for i = 2:size(setIDs) 
    ID = setIDs(i);
    % generate a mask with the ID we want
    mask = (sequence == ID);
    filename = 'mask_';
    dims = size(mask(1, 1, :))
    slices = dims(3);
    folderName = strcat(filename, num2str(ID), '_stack')
    mkdir(folderName);
    % iterate over the images and write them in a folder as pngs
    for j = 1:slices
        imageName = strcat(folderName,'/', num2str(j-1), '.png')
        imwrite(mask(:, :, j), imageName);
    end
end


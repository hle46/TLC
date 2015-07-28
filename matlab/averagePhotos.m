function [] = averagePhotos( folder, n)
    image = im2double(imread(fullfile(folder, strcat(int2str(1), '.jpg'))));
    images = zeros(size(image, 1), size(image, 2), size(image, 3), n);
    images(:, :, :, 1) = image;
    for i=2:n
        images(:, :, :, i) = im2double(imread(fullfile(folder, strcat(int2str(i), '.jpg'))));    
    end
    I = mean(images, 4);
    imwrite(I, fullfile(folder, 'avg.png'));
end


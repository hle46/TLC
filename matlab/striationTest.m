
if (~exist(fullfile('data-striation', 'no-focus1', 'before', 'avg.png'), 'file')) 
    averagePhotos(fullfile('data-striation', 'no-focus1', 'before'), 8);
end
IB = imread(fullfile('data-striation', 'no-focus1', 'before', '1.jpg'));

if (~exist(fullfile('data-striation', 'no-focus1', 'after', 'avg.png'), 'file')) 
    averagePhotos(fullfile('data-striation', 'no-focus1', 'after'), 8);
end
IA = imread(fullfile('data-striation', 'no-focus1', 'after', '1.jpg'));

diff = double(IA(:, :, 2)) - double(IB(:, :, 2));
diff = (diff - min(diff(:))) * 3;
figure;
imshow(uint8(diff))
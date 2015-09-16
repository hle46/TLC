clear;
close all;
X_LEFT = 20 + 1;
X_RIGHT = 400;
Y_TOP = 350 + 1;
Y_BOTTOM = 450;
imdir = 'a2-75-95-100/3';

if ~exist(fullfile(imdir, 'sample', 'avg.png'), 'file') 
    averagePhotos(fullfile(imdir, 'sample'), 8);
end
I = im2double(imread(fullfile(imdir, 'sample', 'avg.png')));
figure;
imagesc(imrotate(I, -90));
truesize
[h, w, ~] = size(I);

% G = I(:, :, 2);
% G = rgb2gray(I);
G = rgb2ycbcr(I);
G = G(:, :, 1);
G_ROI = G(Y_TOP:(h - Y_BOTTOM), X_LEFT:(w - X_RIGHT));
figure;
imagesc(imrotate(G_ROI * 255, -90));
truesize
colorbar

[ lines ] = detectLines( G_ROI );
figure, hold off, imagesc(imrotate(G_ROI * 255, -90));
truesize
colorbar
hold on
plot([1 size(G_ROI, 1)], [lines.front, lines.front], '-.', 'Color', 'r', 'LineWidth', 2)
plot([1 size(G_ROI, 1)], [lines.origin, lines.origin], 'Color', 'r', 'LineWidth', 2)
legend('Front Line', 'Origin Line');

lines.front = lines.front + X_LEFT - 1;
lines.origin = lines.origin + X_LEFT - 1;
X_OFFSET_FROM_FRONTLINE = 100;
X_OFFSET_FROM_ORIGINLINE = 100;


G_DOTS = G(Y_TOP:(h - Y_BOTTOM), (lines.front + X_OFFSET_FROM_FRONTLINE):(lines.origin - X_OFFSET_FROM_ORIGINLINE));
figure, imshow(G_DOTS);

% Load background
if ~exist(fullfile(imdir, 'bg', 'avg.png'), 'file') 
    averagePhotos(fullfile(imdir, 'bg'), 8);
end
bg = im2double(imread(fullfile(imdir, 'bg', 'avg.png')));

% bgG = bg(:, :, 2);
% bgG = rgb2gray(bg);
bgG = rgb2ycbcr(bg);
bgG = bgG(:, :, 1);
bgG = bgG(Y_TOP:(h - Y_BOTTOM), (lines.front + X_OFFSET_FROM_FRONTLINE):(lines.origin - X_OFFSET_FROM_ORIGINLINE));

figure;
imshow(bgG);
% f = conv2(G_DOTS, fspecial('gaussian', 19, 3), 'valid') ./ conv2(bgG, fspecial('gaussian', 19, 3), 'valid');


f = G_DOTS ./ bgG;
figure;
imagesc(imrotate(f, -90));
truesize;
colorbar

[ mask ] = detectDotsMethod3( f );
labels = bwlabel(mask);
% end
figure;
imagesc(imrotate(mask, -90));
truesize;
colorbar

dots = cell(3, 1);
dotsYMin = zeros(3, 1);
for i=1:3
    [y, x] = find(labels == i);
    yMin = min(y);
    yMax = max(y);
    xMin = min(x);
    xMax = max(x);
    dots{i}.data = f(yMin:yMax, xMin:xMax);
    dots{i}.xMin = xMin;
    dots{i}.xMax = xMax;
    dots{i}.yMin = yMin;
    dots{i}.yMax = yMax;
    dotsYMin(i) = yMin;
end

[dotsYMin, idx] = sort(dotsYMin, 'descend');
dots = dots(idx);

for i=1:3
    dots{i}.ySum = sum(dots{i}.data, 2);
    dots{i}.pY = polyfit((dots{i}.yMin:dots{i}.yMax)', dots{i}.ySum(:), 2);
    dots{i}.yc = findMinPoly(dots{i}.pY, dots{i}.yMin, dots{i}.yMax);
    
    dots{i}.xSum = sum(dots{i}.data);
    dots{i}.pX = polyfit((dots{i}.xMin:dots{i}.xMax)', dots{i}.xSum(:), 2);
    dots{i}.xc = findMinPoly(dots{i}.pX, dots{i}.xMin, dots{i}.xMax);
end

% Plot pX
figure;
colors = ['r', 'g', 'b'];
for i=1:3
    scatter(((dots{i}.xMin - dots{i}.xc):(dots{i}.xMax - dots{i}.xc))', dots{i}.xSum, '.', colors(i));
%     scatter((dots{i}.xMin:dots{i}.xMax)', dots{i}.xSum, '.', colors(i))
    hold on;
    plot(((dots{i}.xMin - dots{i}.xc):(dots{i}.xMax - dots{i}.xc))', polyval(dots{i}.pX, (dots{i}.xMin:dots{i}.xMax)'), 'k');
%     plot((dots{i}.xMin:dots{i}.xMax)', polyval(dots{i}.pX, (dots{i}.xMin:dots{i}.xMax)'), 'k')
end

figure;
colors = ['r', 'g', 'b'];
for i=1:3
    scatter(((dots{i}.yMin - dots{i}.yc):(dots{i}.yMax - dots{i}.yc))', dots{i}.ySum, '.', colors(i));
%     scatter(((dots{i}.yMin - 0):(dots{i}.yMax - 0))', dots{i}.ySum, '.', colors(i));
    hold on;
    plot(((dots{i}.yMin - dots{i}.yc):(dots{i}.yMax - dots{i}.yc))', polyval(dots{i}.pY, (dots{i}.yMin:dots{i}.yMax)'), 'k');
%     plot(((dots{i}.yMin - 0):(dots{i}.yMax - 0))', polyval(dots{i}.pY, (dots{i}.yMin:dots{i}.yMax)'), 'k');
end



G_DOTS = I(Y_TOP:(h - Y_BOTTOM), (lines.front + X_OFFSET_FROM_FRONTLINE):(lines.origin - X_OFFSET_FROM_ORIGINLINE), :);
bgG = bg(Y_TOP:(h - Y_BOTTOM), (lines.front + X_OFFSET_FROM_FRONTLINE):(lines.origin - X_OFFSET_FROM_ORIGINLINE), :);
coeffs = zeros(size(bgG));
for i=1:3
    coeffs(:, :, i) = bgG(:, :, i) ./ (bgG(:, :, 1) .^ 2 + bgG(:, :, 2) .^ 2 + bgG(:, :, 3) .^ 2);
end
f = G_DOTS(:, :, 1).*coeffs(:, :, 1) + G_DOTS(:, :, 2).*coeffs(:, :, 2) + G_DOTS(:, :, 3).*coeffs(:, :, 3);
figure;
imagesc(imrotate(f, -90));
truesize;
colorbar

dots_norm = cell(3, 1);
dotsYMinNorm = zeros(3, 1);
for i=1:3
    [y, x] = find(labels == i);
    yMin = min(y);
    yMax = max(y);
    xMin = min(x);
    xMax = max(x);
    dots_norm{i}.data = f(yMin:yMax, xMin:xMax);
    dots_norm{i}.xMin = xMin;
    dots_norm{i}.xMax = xMax;
    dots_norm{i}.yMin = yMin;
    dots_norm{i}.yMax = yMax;
    dotsYMinNorm(i) = yMin;
end

[dotsYMinNorm, idx] = sort(dotsYMinNorm, 'descend');
dots_norm = dots_norm(idx);

g = f;
f = f .* mask;

% figure;
% hist(f(f ~= 0));
figure; hist(f(mask ~= 0))
remain = imhist(f(mask ~= 0));
thr = 0.5 * sum(remain(:));
s = 0;
for i=1:length(remain)
    s = s + remain(i);
    if (s > thr) 
        break;
    end
end
%thresh = multithresh(f(f ~= 0), 1);
thresh = (i-1)/255;
 g = g <= thresh; g = g .* mask; figure; imshow(g);

% thresh = prctile(f(f ~= 0), 70)
% thresh1 = prctile(f(f ~= 0), 0)
% figure; imshow(f <= thresh & f >= thresh1)

for i=1:3
    data = dots_norm{i}.data <= thresh;
%     data = (dots_norm{i}.data <= thresh & dots_norm{i}.data >= thresh1);
    dots_norm{i}.xc = dots{i}.xc;
    dots_norm{i}.yc = dots{i}.yc;
    dots_norm{i}.rf = 1 - (dots_norm{i}.xc + X_OFFSET_FROM_FRONTLINE - 1) / (lines.origin - lines.front + 1);
    dots_norm{i}.darkness = sum(data(:));
end

figure;
imshow(I);
hold on, plot([lines.origin, lines.origin], [Y_TOP, h - Y_BOTTOM], [lines.front, lines.front], [Y_TOP, h - Y_BOTTOM], 'LineWidth', 2)

figure;
num_pixels = 400;
for i=1:3
    %plotDot(dots_norm{i}, lines.front + X_OFFSET_FROM_FRONTLINE, Y_TOP, lines.origin, colors(i));
    temp = sort(dots_norm{i}.data(:));
    dots_norm{i}.f = temp(1:num_pixels);
    %sum(temp(1:400))
    plot(1:num_pixels, temp(1:num_pixels), colors(i))
    hold on;
%     f = fit((1:num_pixels)', temp(1:num_pixels), 'power2')
%     hold on;
%     plot(f, colors(i));
    dots_norm{i}.darkness
end
legend('75', '95', '100');
xlabel('Index')
ylabel('Relative Brightness')


c = [100/100, 50/100, 75/100];
y2 = zeros(num_pixels, 1);
for i=1:num_pixels
    y1 = dots_norm{1}.f(i); 
    y3 = dots_norm{3}.f(i); 
    k = log(y1 / y3) / (c(3) - c(1));
    N = y1 * exp(k * c(1));
    y2(i) = -log(dots_norm{2}.f(i)/N)/k;
end
%figure; plot(dots_norm{2}.f, '--');
%hold on;
figure;
plot(y2);
% figure;
% p = polyfit(dots_norm{1}.f(10:400), dots_norm{2}.f(10:400), 1)
% y = polyval(p, dots_norm{1}.f(10:400));
% scatter(dots_norm{1}.f, dots_norm{2}.f);
% hold on;
% plot(dots_norm{1}.f(10:400), y);

% [m, b] = ransacfit(dots_norm{1}.f, dots_norm{2}.f);


% figure; plot([0.75 0.9 1], [dots_norm{3}.darkness, dots_norm{2}.darkness, dots_norm{1}.darkness]);
% A = [75 -dots_norm{1}.p; 100 -dots_norm{3}.p]
% b = [75*dots_norm{1}.p; 100*dots_norm{3}.p]
% x = A \ b
% v = dots_norm{2}.darkness / dots_norm{1}.darkness
% (x(2)*dots_norm{2}.p) / (x(1) - dots_norm{2}.p)
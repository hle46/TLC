function [ mag, theta ] = gradientMagnitude( im, sigma )
    % im: RGB image
    % sigma: std of Gaussian filter
     im = im2double(im);
    im_size = size(im);
    assert(im_size(3) == 3);
    gx = zeros(im_size);
    gy = zeros(im_size);
    h_len = ceil(6 * sigma);
    if (mod(h_len, 2) == 0) 
        h_len = h_len + 1;
    end
    h = fspecial('gaussian', h_len, sigma);
    diff_x = [-1 0 1];
    diff_y = [-1; 0; 1];
    for i=1:3
        f = imfilter(im(:, :, i), h);
        gx(:, :, i) = imfilter(f, diff_x);
        gy(:, :, i) = imfilter(f, diff_y);
    end
    mag3 = zeros(im_size(1), im_size(2), 3);
    for i=1:3
        mag3(:, :, i) = gx(:, :, i) .* gx(:, :, i) + gy(:, :, i) .* gy(:, :, i);
    end
    [~, I] = max(mag3, [], 3);
    theta = zeros(im_size(1), im_size(2));
    for i=1:im_size(1)
        for j=1:im_size(2)
            theta(i, j) = atan2(-gy(i, j, I(i, j)), gx(i, j, I(i, j)));
        end
    end
    mag = sqrt(mag3(:, :, 1) + mag3(:, :, 2) + mag3(:, :, 3));
end


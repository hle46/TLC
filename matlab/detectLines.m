function [ lines ] = detectLines( im )
    % Assume lines are straight lines (horizontal or vertical depends on
    % how you interpret it
    % 2D horizontal gradient
    % Gaussian filter the image first, then take second derivative
    sigma = 4;
    % Use at least 6 sigma and be odd
    hSize = 6*sigma + 1;
    dX = conv2(conv2(im, fspecial('gaussian', hSize, 4), 'valid'), [1 -2 1], 'valid');
    % Plus 1 for second derivative
    offset = 3*sigma + 1;
    % 1D gradient
    dx = sum(dX);
    figure;
    plot(dx * 255);
    [~, locs] = findpeaks(dx, 'SortStr', 'descend', 'MinPeakHeight', 0.05);
    if (locs(1) > locs(2))
        lines.front = locs(2) + offset;
        lines.origin = locs(1) + offset;
    else
        lines.front = locs(1) + offset;
        lines.origin = locs(2) + offset;
    end
end


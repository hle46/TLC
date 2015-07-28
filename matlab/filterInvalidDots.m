function [ res ] = filterInvalidDots( mask )
    thr = 20;
    [h, w] = size(mask);
    labels = bwlabel(mask);
    res = mask;
    for i = 1:max(labels(:))
        [y, x] = find(labels == i);
        yMin = min(y);
        yMax = max(y);
        xMin = min(x);
        xMax = max(x);
        if (abs(yMin - 1) < thr || abs(yMax - h) < thr || abs(xMin - 1) < thr || abs(xMax - w) < thr)
            res(labels == i) = 0;
        end
    end
end


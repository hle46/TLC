function [ g ] = spatialThresholds( f, xBins, yBins )
    [h, w] = size(f);
    g = zeros(size(f));
    for i=1:xBins
        for j=1:yBins
            xLow = floor(w * (i - 1) / xBins );
            xHigh = floor(w * i/ xBins);
            yLow = floor(h  * (j - 1)/yBins);
            yHigh = floor(h  * j/yBins);
            thresh = multithresh(f(yLow+1:yHigh, xLow+1:xHigh), 2);
            g(yLow+1:yHigh, xLow+1:xHigh) = f(yLow+1:yHigh, xLow+1:xHigh) <= thresh(1);
        end
    end
end


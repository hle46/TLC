function [ mask ] = detectDotsMethod3( f )
    h = imhist(f);
    thr = 0.04 * sum(h(:));
    s = 0;
    for i=1:256
        s = s + h(i);
        if (s > thr) 
            break;
        end
    end
    mask = f <= (i - 1)/255;
    mask = imopen(mask, strel('disk', 17));
    %mask = imdilate(mask, strel('disk', 5));
    [ mask ] = filterInvalidDots( mask );
end


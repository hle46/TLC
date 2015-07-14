function [ mask ] = detectDotsMethod1( f )
    thresh = graythresh(f);
    mask = f <= thresh;
    mask = imopen(mask, strel('disk', 15));
end


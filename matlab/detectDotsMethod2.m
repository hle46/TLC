function [ mask ] = detectDotsMethod2( f )
    thresh = multithresh(f, 2);
    mask = f <= thresh(1);
%     mask = f <= 0.7;
%     mask = imclose(imopen(mask, strel('disk', 15)), strel('disk', 21));
    mask = imclose(mask, strel('disk', 21));
    [ mask ] = filterInvalidDots( mask );
    
    mask = imopen(mask, strel('disk', 15));
end


function [ minP ] = findMinPoly( p, x1, x2 )
    k = polyder(p);
    r = roots(k);
    r = r(r == real(r));
    r = r(r >= x1 & r <= x2);
    % Assume the range filter out all the falsity 
    minP = r(1);
end


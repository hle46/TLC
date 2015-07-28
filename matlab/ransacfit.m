function [m, b] = ransacfit(x, y)
% y = mx + b
N = 500;
thresh = 0.001;

bestcount = 0;

for k = 1:N
    rp = randperm(numel(x));
    tx = x(rp(1:2));
    ty = y(rp(1:2));
    m = (ty(2)-ty(1)) ./ (tx(2)-tx(1));
    b = ty(2)-m*tx(2);
    
    nin = sum(abs(y-m*x-b)<thresh);
    if nin > bestcount
        bestcount = nin;
        inliers = (abs(y - m*x - b) < thresh);
    end
end

[m, b] = total_lsqfit(x(inliers), y(inliers));
figure, hold off, plot(x, y, 'r*'), hold on, plot(x(inliers), y(inliers), 'g*')
end

function [m, b, err] = total_lsqfit(x, y)
% ax + by + c = 0
% distance to line for (a^2+b^2=1): dist_sq = (ax + by + c).^2
A = [x(:)-mean(x) y(:)-mean(y)];
[v, d] = eig(A'*A);
p = v(:, 1); % eigenvector corr. to smaller eigenvalue

% get a, b, c parameters
a = p(1);
b = p(2);
c = -(a*mean(x)+b*mean(y));
err = (a*x+b*y+c).^2;

% convert to slope-intercept (m, b)
m = -a/b;
b = -c/b; % note: this b is for slope-intercept now
end


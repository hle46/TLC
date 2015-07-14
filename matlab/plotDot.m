function plotDot( dot, offsetX, offsetY, originLine, color )
    scatter(dot.xc + offsetX, dot.yc + offsetY, 9, 'filled', 'MarkerFaceColor', color);
    rectangle('Position',[dot.xMin + offsetX, dot.yMin + offsetY, dot.xMax - dot.xMin + 1, dot.yMax - dot.yMin + 1], 'EdgeColor', color);
    plot([dot.xc + offsetX, originLine], [dot.yc + offsetY, dot.yc + offsetY], color)
    %annotation('textbox',[(dot.xMax + offsetX)/2000, (dot.yMin + offsetY + 10)/1000,  20, 20], 'String', strcat('Darkness: ', dot.darkness, 'rf: ', dot.rf));
end


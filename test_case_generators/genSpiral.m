function genSpiral(sizes)

for i = 1:length(sizes);
    dim = sizes(i);
    x = spiral(dim, dim);
    dlmwrite(sprintf('spiral_%dx%d.dlm', dim, dim), x, ' ');
end
end

function x = spiral(r, c)
x = -1 * ones(r, c);
p = [2 2];

direction = 'right';
rowStart = 1;
rowEnd = r;
colStart = 1;
colEnd = c;


while true
    x(p(1), p(2)) = 1;
    
    switch direction
        case 'right'
            p = p + [0, 1];
            if p(2) == colEnd
                p = p + [1, -1];
                rowStart = p(1);
                if p(1) >= rowEnd || p(2) <= colStart
                    break;
                end
                direction = 'down';
            end
        case 'down'
            p = p + [1, 0];
            if p(1) == rowEnd
                p = p + [-1, -1];
                colEnd = p(2);
                if p(2) <= colStart || p(1) <= rowStart
                    break;
                end
                direction = 'left';
            end
        case 'left'
            p = p + [0, -1];
            if p(2) == colStart
                p = p + [-1, 1];
                rowEnd = p(1);
                if p(1) <= rowStart || p(2) >= colEnd
                    break;
                end
                direction = 'up';
            end
        case 'up'
            p = p + [-1, 0];
            if p(1) == rowStart
                p = p + [1, 1];
                colStart = p(2);
                if p(2) >= colEnd || p(1) >= rowEnd
                    break;
                end
                direction = 'right';
            end
        otherwise
            error('Incorrect direction');
    end
end
end
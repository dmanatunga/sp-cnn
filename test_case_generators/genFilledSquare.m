function genFilledSquare(sizes)

for i = 1:length(sizes);
    dim = sizes(i);
    x = -1 * ones(dim, dim);
    x(2:end-1, 2:end-1) = 1;
    dlmwrite(sprintf('filledSquare_%dx%d.dlm', dim, dim), x, ' ');
end
end
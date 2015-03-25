function genVertLine(sizes)

for i = 1:length(sizes);
    dim = sizes(i);
    x = -1 * ones(dim, dim);
    x(:, 2) = 1;
    dlmwrite(sprintf('vertLine_%dx%d.dlm', dim, dim), x, ' ');
end
end
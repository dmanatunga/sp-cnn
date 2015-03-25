test_dims = 128;
N_vals = 50;
dim_vals = 128;
dt = 1;

testDir = '../../tests/';
gene = @HoleFilling_gene;

for i = 1:length(test_dims)
    if ~any(test_dims(i) == dim_vals)
        error('Unknown dimension');
    end
end
for i = 1:length(test_dims)
    dim = test_dims(i);
    N = N_vals(dim == dim_vals);
    files = dir(sprintf('%s/*_%dx%d.dlm', testDir, dim, dim));
    
    for j = 1:length(files)
        img_file = sprintf('%s/%s', testDir, files(j).name);
        img = dlmread(img_file);
        dim = size(img);
        [r, A, B, z, u, initState ] = gene(img);
        
        [~, final_output] = cnn2D(dim, r, A, B, z, initState, u, dt, N);
        dlmwrite(['out_', files(j).name], final_output, ' ');
    end
end
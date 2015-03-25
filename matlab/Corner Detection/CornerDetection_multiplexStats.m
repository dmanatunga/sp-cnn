function CornerDetection_multiplexStats(testDir, test_dims, cnn_ratio, partition_schemes)

N_vals = [32, 32, 32, 32];
dim_vals = [16, 32, 64, 128];
dt = 1;

gene = @CornerDetection_gene;

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
        test = sprintf('%s/%s', testDir, files(j).name);
        for k = 1:length(partition_schemes)
            scheme = partition_schemes{k};
            fprintf('Evaluating Test %s - Partition %s...\n', test, scheme);
            gen_multiplex_stats(test, gene, [dim/cnn_ratio, dim/cnn_ratio], N, dt, scheme, 'results');
        end
    end
end
end
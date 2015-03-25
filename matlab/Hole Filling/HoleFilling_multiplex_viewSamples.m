function HoleFilling_multiplex_viewSamples( img_file, cnn_dim, cnn_N, partition_N, dt, partition_type)
%HOLEFILLING__MULTIPLEX_VIEWSAMPLES

img = dlmread(img_file);
dim = size(img);
[r, A, B, z, u, initState ] = HoleFilling_gene(img);
partitions = gen_partitions2D(dim, cnn_dim, partition_type);

samples = cnn2D_multiplex(cnn_dim, r, A, B, z, initState, u, ... 
                          dt, cnn_N, partition_N, partitions, 'sample-iter');
cnn_info = struct('N', cnn_N * partition_N, 'dt', dt, 'input', img);
viewResults('samples', cnn_info, samples);
end
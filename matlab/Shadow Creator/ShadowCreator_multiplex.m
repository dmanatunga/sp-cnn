function ShadowCreator_multiplex(img_file, cnn_dim, cnn_N, partition_N, dt, partition_type)
img = dlmread(img_file);
dim = size(img);

[r, A, B, z, u, initState ] = HoleFilling_gene(img);

partitions = gen_partitions2D(dim, cnn_dim, partition_type);

[~, final_output] = cnn2D_multiplex(cnn_dim, r, A, B, z, initState, u, ... 
                                    dt, cnn_N, partition_N, partitions);
                                
viewResults('side-by-side', img, final_output);                                
end
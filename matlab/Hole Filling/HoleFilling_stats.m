function HoleFilling_stats( img_file, cnn_dim, N, dt, partition_type, results_dir)
gene = @HoleFilling_gene;
gen_multiplex_stats(img_file, gene, cnn_dim, N, dt, partition_type, results_dir);
end


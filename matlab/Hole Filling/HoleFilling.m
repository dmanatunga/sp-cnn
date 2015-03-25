function HoleFilling(img_file, N, dt)
%HOLEFILLING Invokes CNN to Hole-Fill on input image

img = dlmread(img_file);
dim = size(img);
[r, A, B, z, u, initState ] = HoleFilling_gene(img);

t_final = N * dt;

[~, final_output] = cnn2D(dim, r, A, B, z, initState, u, dt, N);
title1 = sprintf('Initial Image (N=%d, t_f=%.2f, dt=%.3f)', N, t_final, dt);
title2 = sprintf('Final Output: Sample %d, Time %.3f', N, N * dt);
viewResults('side-by-side', img, final_output, 'title', {title1, title2});
end
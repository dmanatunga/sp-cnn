function GlobalConnect1pix(img_file, N, dt)
%GLOBALCONNECT1PIX Summary of this function goes here
%   Detailed explanation goes here
img = dlmread(img_file);
dim = size(img);
[r, A, B, z, u, initState ] = GlobalConnect1pix_gene(img);

t_final = N * dt;

[~, final_output] = cnn2D(dim, r, A, B, z, initState, u, dt, N);
title1 = sprintf('Initial Image (N=%d, t_f=%.2f, dt=%.3f)', N, t_final, dt);
title2 = sprintf('Final Output: Sample %d, Time %.3f', N, N * dt);
viewResults('side-by-side', img, final_output, 'title', {title1, title2});
end


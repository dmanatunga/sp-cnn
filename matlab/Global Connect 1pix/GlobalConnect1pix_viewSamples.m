function GlobalConnect1pix_viewSamples( img_file, N, dt, num_samples)
%GLOBALCONNECT1PIX_VIEWSAMPLES

img = dlmread(img_file);
dim = size(img);
[r, A, B, z, u, initState ] = GlobalConnect1pix_gene(img);

samples = cnn2D(dim, r, A, B, z, initState, u, dt, N, 'sample-interval', num_samples);
cnn_info = struct('N', N, 'dt', dt, 'input', img);
viewResults('samples', cnn_info, samples);
end
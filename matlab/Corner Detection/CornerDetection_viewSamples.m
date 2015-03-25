function CornerDetection_viewSamples( img_file, N, dt, num_samples)
%CornerDetection_VIEWSAMPLES

img = dlmread(img_file);
dim = size(img);
[r, A, B, z, u, initState ] = CornerDetection_gene(img);

samples = cnn2D(dim, r, A, B, z, initState, u, dt, N, 'sample-interval', num_samples);
cnn_info = struct('N', N, 'dt', dt, 'input', img);
viewResults('samples', cnn_info, samples);
end
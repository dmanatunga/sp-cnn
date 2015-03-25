function ShadowCreator_movie(img_file, N, dt)
%HOLEFILLING Invokes CNN to Hole-Fill on input image

img = dlmread(img_file);
dim = size(img);
[r, A, B, z, u, initState ] = ShadowCreator_gene(img);
cnnAnimation(img_file, dim, r, A, B, z, initState, u, dt, N);
end
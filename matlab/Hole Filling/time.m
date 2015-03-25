N = 1000;
dt = 1;
dims = [64 128 256 512 1024];
times = zeros(1, length(dims));
for i = 1:length(dims)
    img = dlmread(sprintf('../../tests/spiral_%dx%d.dlm', dims(i), dims(i)));
    dim = size(img);
    [r, A, B, z, u, initState ] = HoleFilling_gene(img);

    t_final = N * dt;
    x = 0;
    for t = 1:3
        tic;
        [~, final_output] = cnn2D(dim, r, A, B, z, initState, u, dt, N);
        x = x + toc;
    end
    times(i) = x / 3;
end
dlmwrite('times.dlm', times, ',');

% title1 = sprintf('Initial Image (N=%d, t_f=%.2f, dt=%.3f)', N, t_final, dt);
% title2 = sprintf('Final Output: Sample %d, Time %.3f', N, N * dt);
% viewResults('side-by-side', img, final_output, 'title', {title1, title2});

% 222.8587
% 223.2297
%
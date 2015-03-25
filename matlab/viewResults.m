function viewResults(mode, varargin)
switch mode
    case 'side-by-side'
        in1 = varargin{1};
        in2 = varargin{2};
        viewSideBySide(in1, in2, varargin{3:end});
    case 'samples'
        viewSamples(varargin{1}, varargin{2});
end
end

function viewSideBySide(in1, in2, varargin)
warning off;
g1 = subplot(1, 2, 1);
visualizeImage(in1, 'grid');

g2 = subplot(1, 2, 2);
visualizeImage(in2, 'grid');

for i = 1:2:length(varargin)
    switch varargin{i}
        case 'title'
            titles = varargin{i+1};
            title(g1, titles{1});
            title(g2, titles{2});
    end
end
warning on;
end

function viewSamples(cnn_info, samples)
img = cnn_info.input;
N = cnn_info.N;
dt = cnn_info.dt;
num_samples = length(samples);
num_cols = 5;
num_rows = ceil(num_samples / num_cols);
num_cols = num_cols + 1;
warning off;
subplot(num_rows, num_cols, 1:num_cols:num_samples+1);
visualizeImage(img, 'grid');
title(sprintf('Initial Image (N=%d, dt=%.3f)', N, dt));
for i = 1:num_samples
    subplot(num_rows, num_cols, num_cols * floor((i - 1) / (num_cols - 1)) + 1 + mod((i - 1), num_cols - 1) + 1);
    visualizeImage(samples(i).output, 'grid');
    title(sprintf('Sample %d, Time %.3f', samples(i).iter, samples(i).time));
end;
warning on;
end
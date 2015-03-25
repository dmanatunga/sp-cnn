function cnnAnimation(inital_imgfile, dim, r, A, B, z, initState, u, dt, N)
NUM_FRAMES_PER_ITER = round(20 * 0.75);

samples = cnn2D( dim, r, A, B, z, initState, u, dt, N, 'sample-interval', N+1);

image_size = getImageSize(dim);
frames = zeros(image_size(1), image_size(2), 1, NUM_FRAMES_PER_ITER * (length(samples)+1));

img = genImage(inital_imgfile, 'space');
for j = 1:NUM_FRAMES_PER_ITER
    frames(:, :, :, j) = img;
end

for i = 1:N+1
    img = genImage(samples(i).output, 'space');
    for j = 1:NUM_FRAMES_PER_ITER
        frames(:, :, :, i * NUM_FRAMES_PER_ITER + j) = img;
    end
end
map = [0 0 0; 1 1 1];
mov = immovie(frames, map);
implay(mov);

obj = VideoWriter('HoleFillingMovie');
open(obj);
writeVideo(obj, frames-1);
close(obj);

end

function ret = getImageSize(dim)
CELL_WIDTH = 30;
ret = dim * CELL_WIDTH;
end

function img = genImage( in, varargin )
%VISUALIZEIMG Summary of this function goes here
%   Detailed explanation goes here
if (length(varargin) == 1)
    mode = varargin{1};
else
    mode = 'space';
end

if (ischar(in))
    arr = dlmread(in);
elseif (isnumeric(in))
    arr = in;
end

[r, c] = size(arr);
switch mode
    case 'space'
        CELL_WIDTH = 30;
        BOUNDARY_WIDTH = 3;
        MAX_SQUARE_WIDTH = CELL_WIDTH - 2 * BOUNDARY_WIDTH;
        img = ones(r * CELL_WIDTH, c * CELL_WIDTH);
        
        for i = 1:r
            for j = 1:c
                cell_val = arr(i, j);
                square_width = (cell_val + 1) / 2 * MAX_SQUARE_WIDTH;
                square_start = (CELL_WIDTH - square_width) / 2;
                cell_i = (i - 1) * CELL_WIDTH + 1 + square_start;
                cell_j = (j - 1) * CELL_WIDTH + 1 + square_start;
                img(cell_i:cell_i+square_width-1, cell_j:cell_j+square_width-1) = 0;
            end
        end
    case 'grid'
        CELL_WIDTH = 53;img = genImage(inital_img, 'space');
for j = 1:NUM_FRAMES_PER_ITER
    frames(:, :, :, j) = img;
end
        BOUNDARY_WIDTH = 3;
        MAX_SQUARE_WIDTH = CELL_WIDTH - BOUNDARY_WIDTH;
        img = ones(r * CELL_WIDTH + BOUNDARY_WIDTH, c * CELL_WIDTH + BOUNDARY_WIDTH);
        
        
        for i = 0:r-1
            ind1 = CELL_WIDTH * i + 1;
            img(ind1:ind1 + BOUNDARY_WIDTH - 1, :)  = 0;
        end
        img(end - BOUNDARY_WIDTH + 1:end, :) = 0;
        
        for i = 0:c-1
            ind1 = CELL_WIDTH * i + 1;
            img(:, ind1:ind1 + BOUNDARY_WIDTH - 1)  = 0;
        end
        img(:, end - BOUNDARY_WIDTH + 1:end) = 0;
        
        for i = 1:r
            for j = 1:c
                cell_val = arr(i, j);
                square_width = (cell_val + 1) / 2 * MAX_SQUARE_WIDTH;
                square_start = (CELL_WIDTH - square_width + BOUNDARY_WIDTH) / 2;
                cell_i = (i - 1) * CELL_WIDTH + 1 + square_start;
                cell_j = (j - 1) * CELL_WIDTH + 1 + square_start;
                img(cell_i:cell_i+square_width-1, cell_j:cell_j+square_width-1) = 0;
            end
        end
    otherwise
        error('Invalid mode entered');
end
img = img + 1;
end
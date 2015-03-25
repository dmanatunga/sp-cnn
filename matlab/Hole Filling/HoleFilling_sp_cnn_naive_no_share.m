function HoleFilling_sp_cnn_naive_no_share()

cnn_dim = [128, 128];
test_dir = '../../test_files/';
tests = {'lowerA', 'capitalA', 'vertLine', 'circle', 'rect', 'filledSquare', 'zero', 'seven', 'eight', 'nine'};
%tests = {'filledSquare'};
raw = cell(length(tests) + 1, 3);
raw(1, 1:3) = {'Test', 'Total Conv Time', 'Virtual Conv Time'};
for i = 1:length(tests)
    test_name = tests{i};
    disp(test_name);
    img_file = [test_dir, test_name, '_512x512.dlm'];
    [tot_conv_time, virt_conv_time] = HoleFilling_naive(test_name, img_file, cnn_dim);
    raw{i+1, 1} = test_name;
    raw{i+1, 2} = tot_conv_time;
    raw{i+1, 3} = virt_conv_time;
end
my_csvwrite('naive_no_share_results/results.csv', raw);
end

function [tot_conv_time, virt_conv_time] = HoleFilling_naive(test_name, img_file, cnn_dim)
img = dlmread(img_file);
dim = size(img);


partition_set = gen_partitions2D(dim, cnn_dim, 'independent');
tot_conv_time = 0;
virt_conv_time = 0;
partitions = partition_set.partitions;

state = zeros(dim);
output = zeros(dim);
for i = 1:length(partitions)
    tmpImage = slice(img, partitions{i}, cnn_dim);
    [r, A, B, z, u, initState ] = HoleFilling_gene(tmpImage);
    [count, cnn_state, cnn_output] = cnn2D_run( cnn_dim, r, A, B, z, initState, u);
    tot_conv_time = tot_conv_time + count;
    
    if (count > virt_conv_time)
        virt_conv_time = count;
    end
    
    state = assign_slice(cnn_state, state, partitions{i}, cnn_dim);
    output = assign_slice(cnn_output, output, partitions{i}, cnn_dim);
end

dlmwrite(['naive_no_share_results/', test_name, '_state.dlm'], state, ' ');
dlmwrite(['naive_no_share_results/', test_name, '_output.dlm'], output, ' ');                           
end

function [count, state, output] = cnn2D_run( dim, r, A, B, z, initState, u)
%CNN_ARRAY Summary of this function goes here
%   dim - Vector for dimensions of array
%   A - matrix A template
%   B - matrix B template
%   z - threshold value

R = dim(1);
C = dim(2);
state = initState;
output = standardOutEq(state);

change = true;
count = 0;
while change
    change = false;
    for i = 1+r:R+r
        for j = 1+r:C+r
            x_ij = state(i, j);
            next_x_ij = digitalStateEq(A, B, ...
                output(i-r:i+r,j-r:j+r), ...
                u(i-r:i+r,j-r:j+r), z);
            state(i, j) = next_x_ij;
            if abs(x_ij - next_x_ij) >= 0.00001
                change = true;
            end
        end
    end
    output = standardOutEq(state);
    count = count + 1;
end
state = state(1+r:end-r, 1+r:end-r);
output = standardOutEq(state);
end

function state = digitalStateEq(A, B, y, u, z)
state = sum(A(:) .* y(:)) + sum(B(:) .* u(:)) + z;
end

function out = standardOutEq(x)
out = (abs(x + 1) - abs(x - 1))./2;
end

function out = slice(array, topLeftPoint, dim)
out = array(topLeftPoint(1):topLeftPoint(1) + dim(1) - 1, ...
    topLeftPoint(2):topLeftPoint(2) + dim(2) - 1);
end

function out = assign_slice(in, out, topLeftPoint, dim)
out(topLeftPoint(1):topLeftPoint(1) + dim(1) - 1, ...
    topLeftPoint(2):topLeftPoint(2) + dim(2) - 1) = in;
end


function [ tot_conv_time, virt_conv_time, state, output ] = sp_cnn_naive( cnn_dim, r, A, B, z, initState, u, ...
    partition_set )

partitions = partition_set.partitions;
state = initState;
old_state = state;

tot_conv_time = 0;
virt_conv_time = 0;
for i = 1:length(partitions)
    cnn_init = slice(old_state, partitions{i}, cnn_dim + 2*r);
    cnn_u = slice(u, partitions{i}, cnn_dim + 2*r);
    [conv_time, cnn_state, ~] = cnn2D_run(cnn_dim, r, A, B, z, cnn_init, cnn_u);
    
    state = assign_slice(cnn_state, state, partitions{i}+r, cnn_dim);
    tot_conv_time = tot_conv_time + conv_time;
    if (conv_time > virt_conv_time)
        virt_conv_time = conv_time;
    end
end

state = state(1+r:end-r, 1+r:end-r);
output = standardOutEq(state);

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
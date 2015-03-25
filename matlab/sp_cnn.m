function [ run_data, state, output ] = sp_cnn(type, cnn_dim, r, A, B, z, initState, u, ...
    partition_set, interval, numUnits )
switch type
    case 'fixed-interval'
        [run_data, state, output] = sp_cnn_fixed(cnn_dim, r, A, B, z, initState, u, ...
            partition_set, interval, numUnits );
    case 'early-finish'
        [run_data, state, output] = sp_cnn_early_finish(cnn_dim, r, A, B, z, initState, u, ...
            partition_set, interval, numUnits );
end
end

function [ run_data, state, output ] = sp_cnn_fixed(cnn_dim, r, A, B, z, initState, u, ...
    partition_set, interval, numUnits)

partitions = partition_set.partitions;
state = initState;
old_state = state;

tot_conv_time = 0;
virt_conv_time = 0;
curUnit = 1;
for n = 1:500
    change = false;
    for i = 1:length(partitions)
        fprintf('Iteration %d - Partition %d', n, i);
        cnn_init = slice(old_state, partitions{i}, cnn_dim + 2*r);
        cnn_u = slice(u, partitions{i}, cnn_dim + 2*r);
        [partition_change, cnn_state, ~] = cnn2D_run_fixed(cnn_dim, r, A, B, z, cnn_init, cnn_u, interval);
        if partition_change
            fprintf('- true\n');
        else
            fprintf('- false\n');
        end
        change = change | partition_change;
        state = assign_slice(cnn_state, state, partitions{i}+r, cnn_dim);
        
        if curUnit == numUnits
            curUnit = 1;
            tot_conv_time = tot_conv_time + interval;
        else
            curUnit = curUnit + 1;
        end
    end
    
    old_state = state;
    
    if curUnit ~= 1
        curUnit = 1;
        tot_conv_time = tot_conv_time + interval;
    end
    
    virt_conv_time = virt_conv_time + interval;
    
    if ~change
        break;
    end
end

state = state(1+r:end-r, 1+r:end-r);
output = standardOutEq(state);

run_data.numIters = n;
run_data.virt_conv_time = virt_conv_time;
run_data.tot_conv_time = tot_conv_time;

end

function [ run_data, state, output ] = sp_cnn_early_finish(cnn_dim, r, A, B, z, initState, u, ...
    partition_set, interval, numUnits)

partitions = partition_set.partitions;
state = initState;
old_state = state;

tot_conv_time = 0;
virt_conv_time = 0;
unit_conv_times = zeros(1, numUnits);
overall_partition_conv_times = zeros(1, length(partitions));

for n = 1:500
    change = false;
    partition_conv_times = zeros(1, length(partitions));
    for i = 1:length(partitions)
        fprintf('Iteration %d - Partition %d', n, i);
        cnn_init = slice(old_state, partitions{i}, cnn_dim + 2*r);
        cnn_u = slice(u, partitions{i}, cnn_dim + 2*r);
        [~, unit_ind] = min(unit_conv_times);
        [partition_count, cnn_state, ~] = cnn2D_run_conv(cnn_dim, r, A, B, z, cnn_init, cnn_u, interval);
        partition_change = partition_count > 1;
        if partition_change
            fprintf('- true\n');
        else
            fprintf('- false\n');
        end
        change = change | partition_change;
        state = assign_slice(cnn_state, state, partitions{i}+r, cnn_dim);
        
        partition_conv_times(i) = partition_count;
        unit_conv_times(unit_ind) = unit_conv_times(unit_ind) + partition_count;
    end
    
    old_state = state;

    overall_partition_conv_times = overall_partition_conv_times + partition_conv_times;
    virt_conv_time = virt_conv_time + max(partition_conv_times);
    tot_conv_time = max(unit_conv_times);
    
    if ~change
        break;
    end
end

state = state(1+r:end-r, 1+r:end-r);
output = standardOutEq(state);

run_data.numIters = n;
run_data.virt_conv_time = virt_conv_time;
run_data.tot_conv_time = tot_conv_time;
run_data.partition_conv_times = overall_partition_conv_times;
run_data.unit_conv_times = unit_conv_times;

end

function [change, state, output] = cnn2D_run_fixed( dim, r, A, B, z, initState, u, intervalLimit)
%CNN_ARRAY Summary of this function goes here
%   dim - Vector for dimensions of array
%   A - matrix A template
%   B - matrix B template
%   z - threshold value

R = dim(1);
C = dim(2);
state = initState;
output = standardOutEq(state);

change = false;
for k = 1:intervalLimit
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
end
state = state(1+r:end-r, 1+r:end-r);
output = standardOutEq(state);
end

function [count, state, output] = cnn2D_run_conv( dim, r, A, B, z, initState, u, intervalLimit)
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
while change && (count < intervalLimit)
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
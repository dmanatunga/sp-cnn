function varargout = cnn2D_multiplex( cnn_dim, r, A, B, z, initState, u, ...
    dt, cnn_N, partition_N, partition_set, varargin )
%CNN2D_MULTIPLEX Summary of this function goes here
%   dim - Vector for dimensions of array
%   A - matrix A template
%   B - matrix B template
%   z - threshold value
num_args = length(varargin);
if num_args == 0
    [state, output] = cnn2D_multiplex_run( cnn_dim, r, A, B, z, initState, u, ...
        dt, cnn_N, partition_N, partition_set );
    varargout{1} = state;
    varargout{2} = output;
elseif num_args == 1
    switch varargin{1}
        case 'sample-iter'
            samples = cnn2D_multiplex_sampleIter( cnn_dim, r, A, B, z, initState, u, ...
                dt, cnn_N, partition_N, partition_set );
            varargout{1} = samples;
        otherwise
            error('Invalid parameter.');
    end
end
end

function [ state, output ] = cnn2D_multiplex_run( cnn_dim, r, A, B, z, initState, u, ...
    dt, cnn_N, partition_N, partition_set )
%CNN2D_MULTIPLEX Summary of this function goes here
%   Detailed explanation goes here
switch partition_set.type
    case 'dependent'
        partitions = partition_set.partitions;
        state = initState;
        output = zeros(size(initState - r));
        for t = 1:partition_N
            for i = 1:length(partitions)
                cnn_init = slice(state, partitions{i}, cnn_dim + 2*r);
                cnn_u = slice(u, partitions{i}, cnn_dim + 2*r);
                [cnn_state, cnn_out] = cnn2D(cnn_dim, r, A, B, z, cnn_init, cnn_u, dt, cnn_N);
                
                state = assign_slice(cnn_state, state, partitions{i}+r, cnn_dim);
                output = assign_slice(cnn_out, output, partitions{i}+r, cnn_dim);
            end
        end
        state = state(1+r:end-r, 1+r:end-r);
        output = output(1+r:end-r, 1+r:end-r);
    case 'independent'
        partitions = partition_set.partitions;
        state = initState;
        old_state = state;
        output = zeros(size(initState - r));
        for t = 1:partition_N
            for i = 1:length(partitions)
                cnn_init = slice(old_state, partitions{i}, cnn_dim + 2*r);
                cnn_u = slice(u, partitions{i}, cnn_dim + 2*r);
                [cnn_state, cnn_out] = cnn2D(cnn_dim, r, A, B, z, cnn_init, cnn_u, dt, cnn_N);
                
                state = assign_slice(cnn_state, state, partitions{i}+r, cnn_dim);
                output = assign_slice(cnn_out, output, partitions{i}+r, cnn_dim);
            end
            old_state = state;
        end
        state = state(1+r:end-r, 1+r:end-r);
        output = output(1+r:end-r, 1+r:end-r);
    case 'no-share-boundaries'
        partitions = partition_set.partitions;
        state = initState;
        output = zeros(size(initState - r));
        for t = 1:partition_N
            for i = 1:length(partitions)
                cnn_init = zeros(cnn_dim + 2 * r);
                cnn_u = zeros(cnn_dim + 2 * r);
                
                cnn_init(1+r:end-r,1+r:end-r) = slice(state, partitions{i}+r, cnn_dim);
                cnn_u(1+r:end-r,1+r:end-r)  = slice(u, partitions{i}+r, cnn_dim);
                [cnn_state, cnn_out] = cnn2D(cnn_dim, r, A, B, z, cnn_init, cnn_u, dt, cnn_N);
                
                state = assign_slice(cnn_state, state, partitions{i}+r, cnn_dim);
                output = assign_slice(cnn_out, output, partitions{i}+r, cnn_dim);
            end
        end
        state = state(1+r:end-r, 1+r:end-r);
        output = output(1+r:end-r, 1+r:end-r);
end
end

function samples = cnn2D_multiplex_sampleIter( cnn_dim, r, A, B, z, initState, u, ...
    dt, cnn_N, partition_N, partition_set )
%CNN2D_MULTIPLEX Summary of this function goes here
%   Detailed explanation goes here

samples = struct('partition_iter', cell(1, partition_N), ...
    'iter', [], ...
    'time', [], ...
    'state', [], ...
    'output', []);

partitions = partition_set.partitions;
switch partition_set.type
    case 'dependent'
        state = initState;
        output = zeros(size(initState - r));
        
        for t = 1:partition_N
            for i = 1:length(partitions)
                cnn_init = slice(state, partitions{i}, cnn_dim + 2*r);
                cnn_u = slice(u, partitions{i}, cnn_dim + 2*r);
                [cnn_state, cnn_out] = cnn2D(cnn_dim, r, A, B, z, cnn_init, cnn_u, dt, cnn_N);
                
                state = assign_slice(cnn_state, state, partitions{i}+r, cnn_dim);
                output = assign_slice(cnn_out, output, partitions{i}+r, cnn_dim);
            end
            samples(t).partition_iter = t;
            samples(t).iter = t * cnn_N;
            samples(t).time = t * cnn_N * dt;
            samples(t).state = state(1+r:end-r, 1+r:end-r);
            samples(t).output = output(1+r:end-r, 1+r:end-r);
        end
    case 'independent'
        state = initState;
        output = zeros(size(initState - r));
        
        old_state = state;
        for t = 1:partition_N
            for i = 1:length(partitions)
                cnn_init = slice(old_state, partitions{i}, cnn_dim + 2*r);
                cnn_u = slice(u, partitions{i}, cnn_dim + 2*r);
                [cnn_state, cnn_out] = cnn2D(cnn_dim, r, A, B, z, cnn_init, cnn_u, dt, cnn_N);
                
                state = assign_slice(cnn_state, state, partitions{i}+r, cnn_dim);
                output = assign_slice(cnn_out, output, partitions{i}+r, cnn_dim);
            end
            samples(t).partition_iter = t;
            samples(t).iter = t * cnn_N;
            samples(t).time = t * cnn_N * dt;
            samples(t).state = state(1+r:end-r, 1+r:end-r);
            samples(t).output = output(1+r:end-r, 1+r:end-r);
            old_state = state;
        end
    case 'no-share-boundaries'
        state = initState;
        output = zeros(size(initState - r));
        for t = 1:partition_N
            for i = 1:length(partitions)
                cnn_init = zeros(cnn_dim + 2 * r);
                cnn_u = zeros(cnn_dim + 2 * r);
                
                cnn_init(1+r:end-r,1+r:end-r) = slice(state, partitions{i}+r, cnn_dim);
                cnn_u(1+r:end-r,1+r:end-r)  = slice(u, partitions{i}+r, cnn_dim);
                [cnn_state, cnn_out] = cnn2D(cnn_dim, r, A, B, z, cnn_init, cnn_u, dt, cnn_N);
                
                state = assign_slice(cnn_state, state, partitions{i}+r, cnn_dim);
                output = assign_slice(cnn_out, output, partitions{i}+r, cnn_dim);
            end
            samples(t).partition_iter = t;
            samples(t).iter = t * cnn_N;
            samples(t).time = t * cnn_N * dt;
            samples(t).state = state(1+r:end-r, 1+r:end-r);
            samples(t).output = output(1+r:end-r, 1+r:end-r);
        end
end
end


function out = slice(array, topLeftPoint, dim)
out = array(topLeftPoint(1):topLeftPoint(1) + dim(1) - 1, ...
    topLeftPoint(2):topLeftPoint(2) + dim(2) - 1);
end

function out = assign_slice(in, out, topLeftPoint, dim)
out(topLeftPoint(1):topLeftPoint(1) + dim(1) - 1, ...
    topLeftPoint(2):topLeftPoint(2) + dim(2) - 1) = in;
end


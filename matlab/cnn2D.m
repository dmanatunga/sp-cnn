function varargout = cnn2D( dim, r, A, B, z, initState, u, dt, N, varargin)
%CNN_ARRAY Summary of this function goes here
%   dim - Vector for dimensions of array
%   A - matrix A template
%   B - matrix B template
%   z - threshold value
num_args = length(varargin);
if num_args == 0
    [state, output] = cnn2D_run( dim, r, A, B, z, initState, u, dt, N);
    varargout{1} = state;
    varargout{2} = output;
elseif num_args == 2
    switch varargin{1}
        case 'sample-interval'
            num_samples = varargin{2};
            sampling_interval = N / (num_samples - 1);
            sample_list = round(0:sampling_interval:N);
            samples = cnn2D_sample_list(dim, r, A, B, z, initState, u, dt, N, sample_list);
            varargout{1} = samples;
        case 'sample'
            sample_list = varargin{2};
            samples = cnn2D_sample_list( dim, r, A, B, z, initState, u, dt, N, sample_list);
            varargout{1} = samples;
        otherwise
            error('Invalid parameter.');
    end
end
end

function [state, output] = cnn2D_run( dim, r, A, B, z, initState, u, dt, N)
%CNN_ARRAY Summary of this function goes here
%   dim - Vector for dimensions of array
%   A - matrix A template
%   B - matrix B template
%   z - threshold value

R = dim(1);
C = dim(2);
state = initState;
output = standardOutEq(state);

for t = 1:N
    for i = 1+r:R+r
        for j = 1+r:C+r
            x_ij = state(i, j);
            state_der = standardStateEq(A, B, ...
                output(i-r:i+r,j-r:j+r), ...
                u(i-r:i+r,j-r:j+r), z, x_ij);
            state(i, j) = x_ij + state_der .* dt;
        end
    end
    output = standardOutEq(state);
end
state = state(1+r:end-r, 1+r:end-r);
output = output(1+r:end-r, 1+r:end-r);
end


function samples = cnn2D_sampInterval( dim, r, A, B, z, initState, u, dt, N, num_samples)
R = dim(1);
C = dim(2);
state = initState;
output = standardOutEq(state);

sampling_interval = ceil(N / (num_samples - 1));
samples = struct('iter', cell(1, num_samples), ...
                 'time', [], ...
                 'state', [], ...
                 'output', []);
samp_ind = 1;
samples(samp_ind).iter = 0;
samples(samp_ind).time = 0;
samples(samp_ind).state = state;
samples(samp_ind).output = output;
samp_ind = samp_ind + 1;

for t = 1:N
    for i = 1+r:R+r
        for j = 1+r:C+r
            x_ij = state(i, j);
            state_der = standardStateEq(A, B, ...
                output(i-r:i+r,j-r:j+r), ...
                u(i-r:i+r,j-r:j+r), z, x_ij);
            state(i, j) = x_ij + state_der .* dt;
        end
    end
    output = standardOutEq(state);
    
    if mod(t, sampling_interval) == 0
        samples(samp_ind).iter = t;
        samples(samp_ind).time = t * dt;
        samples(samp_ind).input  = state(1+r:end-r, 1+r:end-r);
        samples(samp_ind).output  = output(1+r:end-r, 1+r:end-r);
        samp_ind = samp_ind + 1;
    end
end
if samples(end).iter ~= N
    samples(samp_ind).iter = N;
    samples(samp_ind).time = N * dt;
    samples(samp_ind).input  = state(1+r:end-r, 1+r:end-r);
    samples(samp_ind).output  = output(1+r:end-r, 1+r:end-r);
end
end

function samples = cnn2D_sample_list( dim, r, A, B, z, initState, u, dt, N, sample_list)
R = dim(1);
C = dim(2);
state = initState;
output = standardOutEq(state);

num_samples = length(sample_list);
samples = struct('iter', cell(1, num_samples), ...
                 'time', [], ...
                 'state', [], ...
                 'output', []);
samp_ind = 1;
if (sample_list(samp_ind) == 0)
    samples(samp_ind).iter = 0;
    samples(samp_ind).time = 0;
    samples(samp_ind).input  = state(1+r:end-r, 1+r:end-r);
    samples(samp_ind).output  = output(1+r:end-r, 1+r:end-r);
    samp_ind = samp_ind + 1;
end

for t = 1:N
    for i = 1+r:R+r
        for j = 1+r:C+r
            x_ij = state(i, j);
            state_der = standardStateEq(A, B, ...
                output(i-r:i+r,j-r:j+r), ...
                u(i-r:i+r,j-r:j+r), z, x_ij);
            state(i, j) = x_ij + state_der .* dt;
        end
    end
    output = standardOutEq(state);
    
    if samp_ind <= num_samples && t == sample_list(samp_ind)
        samples(samp_ind).iter = t;
        samples(samp_ind).time = t * dt;
        samples(samp_ind).state  = state(1+r:end-r, 1+r:end-r);
        samples(samp_ind).output  = output(1+r:end-r, 1+r:end-r);
        samp_ind = samp_ind + 1;
    end
end
end


function state_der = standardStateEq(A, B, y, u, z, x_ij)
state_der = -x_ij + sum(A(:) .* y(:)) + sum(B(:) .* u(:)) + z;
end

function out = standardOutEq(x)
out = abs(x + 1)./2 - abs(x - 1)./2;
end
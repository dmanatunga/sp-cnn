function N = EdgeDetectionConverge(img_file, dt)
%EDGEDETECTION Summary of this function goes here
%   Detailed explanation goes here
img = dlmread(img_file);
dim = size(img);
[r, A, B, z, u, initState ] = EdgeDetection_gene(img);

R = dim(1);
C = dim(2);
state = initState;
output = standardOutEq(state);

change = true;
N = 0;
while change
    change = false;
    for i = 1+r:R+r
        for j = 1+r:C+r
            x_ij = state(i, j);
            state_der = standardStateEq(A, B, ...
                output(i-r:i+r,j-r:j+r), ...
                u(i-r:i+r,j-r:j+r), z, x_ij);
            state(i, j) = x_ij + state_der .* dt;
            if (abs(x_ij - state(i, j)) >= 0.00001)
                change = true;
            end
        end
    end
    output = standardOutEq(state);
    N = N + 1;
end
end



function state_der = standardStateEq(A, B, y, u, z, x_ij)
state_der = -x_ij + sum(A(:) .* y(:)) + sum(B(:) .* u(:)) + z;
end

function out = standardOutEq(x)
out = abs(x + 1)./2 - abs(x - 1)./2;
end


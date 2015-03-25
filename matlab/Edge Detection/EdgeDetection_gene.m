function [r, A, B, z, u, initState] = EdgeDetection_gene(in)
%EDGEDETECTION_GENE Summary of this function goes here
%   Detailed explanation goes here
[inR, inC] = size(in);

% Cell radius for CNN
r = 1; 
% A template for Hole-Filling
A = [0 0 0;
     0 0 0;
     0 0 0];
% B template for Hole-Filling
B = [-1 -1 -1;
     -1 8 -1;
     -1 -1 -1];
z = -1; % Threshold

initState = zeros(inR + 2 * r, inC + 2 * r);
u = zeros(inR + 2 * r, inC + 2 * r);
u(1+r:end-r, 1+r:end-r) = in;
end


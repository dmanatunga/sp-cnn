function [r, A, B, z, u, initState] = CornerDetection_gene(in)
%CORNERDETECTION_GENE Summary of this function goes here
%   Detailed explanation goes here
[inR, inC] = size(in);

% Cell radius for CNN
r = 1; 
% A template for Hole-Filling
A = [0 0 0;
     0 1 0;
     0 0 0];
% B template for Hole-Filling
B = [-1 -1 -1;
     -1 4 -1;
     -1 -1 -1];
z = -5; % Threshold

initState = zeros(inR + 2 * r, inC + 2 * r);
u = zeros(inR + 2 * r, inC + 2 * r);
u(1+r:end-r, 1+r:end-r) = in;
end


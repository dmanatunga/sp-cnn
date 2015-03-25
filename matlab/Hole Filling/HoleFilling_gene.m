function [r, A, B, z, u, initState] = HoleFilling_gene(in)
%HOLEFILLING_GENE Summary of this function goes here
%   Detailed explanation goes here
[inR, inC] = size(in);

% Cell radius for CNN
r = 1; 
% A template for Hole-Filling
A = [0 1 0;
     1 2 1;
     0 1 0];
% B template for Hole-Filling
B = [0 0 0;
     0 4 0;
     0 0 0];
z = -1; % Threshold

initState = zeros(inR + 2 * r, inC + 2 * r);
initState(1+r:end-r, 1+r:end-r) = ones(inR, inC);
u = zeros(inR + 2 * r, inC + 2 * r);
u(1+r:end-r, 1+r:end-r) = in;
end


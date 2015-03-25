function [r, A, B, z, u, initState] = RotationDetector_gene(in)
%ROTATIONDETECTOR_GENE Summary of this function goes here
%   Detailed explanation goes here
[inR, inC] = size(in);

% Cell radius for CNN
r = 1; 
% A template for Hole-Filling
A = [-0.8 5 -0.8;
     5 5 5;
     -0.8 5 -0.8];
% B template for Hole-Filling
B = [-0.4 -2.5 -0.4;
     -2.5 5 -2.5;
     -0.4 -2.5 -0.4];
z = -11.2; % Threshold

initState = zeros(inR + 2 * r, inC + 2 * r);
initState(1+r:end-r, 1+r:end-r) = in;
u = zeros(inR + 2 * r, inC + 2 * r);
u(1+r:end-r, 1+r:end-r) = in;
end


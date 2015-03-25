function [r, A, B, z, u, initState] = ShadowCreator_gene(in)
%SHADOWCREATOR_GENE Summary of this function goes here
%   Detailed explanation goes here
[inR, inC] = size(in);

% Cell radius for CNN
r = 1; 
% A template for Hole-Filling
A = [0 0 0;
     0 2 2;
     0 0 0];
% B template for Hole-Filling
B = [0 0 0;
     0 2 0;
     0 0 0];
z = 0; % Threshold

initState = zeros(inR + 2 * r, inC + 2 * r);
initState(1+r:end-r, 1+r:end-r) = ones(inR, inC);
u = zeros(inR + 2 * r, inC + 2 * r);
u(1+r:end-r, 1+r:end-r) = in;
end


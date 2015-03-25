function [r, A, B, z, u, initState] = GlobalConnect1pix_gene(in)
%GLOBALCONNECT1PIX_GENE Summary of this function goes here
%   Detailed explanation goes here
[inR, inC] = size(in);

% Cell radius for CNN
r = 1; 
% A template for Hole-Filling
A = [6 6 6;
     6 9 6;
     6 6 6];
% B template for Hole-Filling
B = [-3 -3 -3;
     -3 9 -3;
     -3 -3 -3];
z = -4.5; % Threshold

initState = zeros(inR + 2 * r, inC + 2 * r);
initState(1+r:end-r, 1+r:end-r) = in;
u = zeros(inR + 2 * r, inC + 2 * r);
u(1+r:end-r, 1+r:end-r) = in;
end


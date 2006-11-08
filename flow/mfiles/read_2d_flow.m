function [u,t] = read_2d_flow( infile )

u = dlmread( infile );
t = u(:,1);
u(:,1) = [];

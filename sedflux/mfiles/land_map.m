function map = land_map( n )

if nargin < 1
   n = size(get(gcf,'colormap'),1);
end

r = [ 0    1   .5  .5 1 ];
g = [  .75 1   .5 0   1 ];
b = [ 0    0  0   0   1 ];

r = interp1([0 .25 .5 .75 1],r,linspace(0,1,n));
g = interp1([0 .25 .5 .75 1],g,linspace(0,1,n));
b = interp1([0 .25 .5 .75 1],b,linspace(0,1,n));

%%%r = interp1([1/n .25 .5 .75 1]*n,r,1:n);
%%%g = interp1([1/n .25 .5 .75 1]*n,g,1:n);
%%%b = interp1([1/n .25 .5 .75 1]*n,b,1:n);

map = [ r' g' b' ];


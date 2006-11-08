function map = water_map( n )

if nargin < 1
   n = size(get(gcf,'colormap'),1);
end

if ( n<=0 )
   map = [];
   return;
end

r = [0 .2 .4 .9];
g = [.1 .4 .6 .9];
b = [.1 .4 .7 1];

if ( n<4 )
   r = r(1:n);
   g = g(1:n);
   b = b(1:n);
else
   r = interp1( [1/n 1/2 2/3 1]*n , r , 1:n , 'linear' )';
   g = interp1( [1/n 1/2 2/3 1]*n , g , 1:n , 'linear' )';
   b = interp1( [1/n 1/2 2/3 1]*n , b , 1:n , 'linear' )';
end

map = [ r g b ];


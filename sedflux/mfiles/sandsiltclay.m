function map = sandsiltclay(m)
% SANDSILTCLAY   sandsiltclay color map.
%    SANDSILTCLAY(M) returns an M-by-3 matrix containing a sandsiltclay
%    colormap.  SANDSILTCLAY, by itself, is the same length as the current
%    colormap.
% 
%    A SANDSILTCLAY colormap is especially useful for displaying sediment
%    properties such as grain size.  The colors vary from yellow, through
%    black, to blue.
%
%    For example, to reset the colormap of the current figure:
% 
%              colormap(sandsiltclay)
% 
%    See also GRAY, HOT, COOL, BONE, COPPER, PINK, FLAG, PRISM, JET,
%    COLORMAP, RGBPLOT, HSV2RGB, RGB2HSV, SEISMIC, STERNSPECIAL.
%

if nargin < 1
   m = size(get(gcf,'colormap'),1);
end

r = [.9 1 .5 0 0];
g = [.9 .5 0 0 0];
b = [0 0 0 .5 1];

r=interp1([1/m .25 .5 .75 1]*m,r,1:m);
g=interp1([1/m .25 .5 .75 1]*m,g,1:m);
b=interp1([1/m .25 .5 .75 1]*m,b,1:m);

map = [ r' g' b' ];

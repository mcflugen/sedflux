function map = sternspecial(m)
% STERNSPECIAL   sternspecial color map.
%    STERNSPECIAL(M) returns an M-by-3 matrix containing a STERNSPECIAL 
%    colormap.
%    STERNSPECIAL, by itself, is the same length as the current colormap.
% 
%    A STERNSPECIAL colormap begins with black, moves through red, blue, 
%    greenish yellow, to white.  It is stolen from the sternspecial colormap
%    used in idl.
% 
%    For example, to reset the colormap of the current figure:
% 
%              colormap(sternspecial)
% 
%    See also GRAY, HOT, COOL, BONE, COPPER, PINK, FLAG, PRISM, JET,
%    COLORMAP, RGBPLOT, HSV2RGB, RGB2HSV, SANDSILTCLAY, SEISMIC.
%

if nargin < 1
   m = size(get(gcf,'colormap'),1);
end

r = [ 0 1 0 .25 1 ];
r = interp1([1/m 1/16 .25 .25+1/m 1]*m,r,1:m);

g = [ 0 1 ];
g = interp1([1/m 1]*m,g,1:m);

b = [ 0 1 0 1];
b = interp1([1/m .5 .75 1]*m,b,1:m);

map = [ r' g' b' ];

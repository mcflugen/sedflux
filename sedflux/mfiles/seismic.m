function map = seismic(m)
% SEISMIC   seismic color map.
%    SEISMIC(M) returns an M-by-3 matrix containing a SEISMIC colormap.
%    SEISMIC, by itself, is the same length as the current colormap.
% 
%    A SEISMIC colormap varies from red, through white, to blue.  The map
%    is particularly useful for displaying seismic data.
%
%    For example, to reset the colormap of the current figure:
% 
%              colormap(seismic)
% 
%    See also GRAY, HOT, COOL, BONE, COPPER, PINK, FLAG, PRISM, JET,
%    COLORMAP, RGBPLOT, HSV2RGB, RGB2HSV, SANDSILTCLAY, STERNSPECIAL.
%

if nargin < 1
   m = size(get(gcf,'colormap'),1);
end

r = [1 1 0];
g = [0 1 0];
b = [0 1 1];

r=interp1([1/m .5 1]*m,r,1:m);
g=interp1([1/m .5 1]*m,g,1:m);
b=interp1([1/m .5 1]*m,b,1:m);

map = [ r' g' b' ];

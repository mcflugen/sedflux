function map = wheeler(m)
% WHEELER   wheeler color map.
%    WHEELER(M) returns an M-by-3 matrix containing a wheeler 
%    colormap.  WHEELER, by itself, is the same length as the current
%    colormap.
% 
%    A WHEELER colormap is especially useful for displaying erosion, deposition 
%    and non-deposition on a Wheeler diagram.  The colormap is three blocks 
%    of color - black to white to gray.
%
%    For example, to reset the colormap of the current figure:
% 
%              colormap(wheeler)
% 
%    See also GRAY, HOT, COOL, BONE, COPPER, PINK, FLAG, PRISM, JET,
%    COLORMAP, RGBPLOT, HSV2RGB, RGB2HSV, SEISMIC, STERNSPECIAL, SANDSILTCLAY.
%

if nargin < 1
   m = size(get(gcf,'colormap'),1);
end

len = floor(m/3);

block1 = [ 0 0 0 ];
block2 = [ 1 1 1 ];
block3 = [ .5 .5 .5 ];

map = [ repmat( block1 , len     , 1 ) ; ...
        repmat( block2 , len     , 1 ) ; ...
        repmat( block3 , m-2*len , 1 ) ];


function map = banded(varargin)
% banded     - Make a colormap with bands.
%
% banded(MAP,N,A) MAP is the colormap to be banded.  If not given, MAP is
%  the colormap of the current figure.  N is the number of bands (default
%  is 5).  A is the degree of banding (default is 0.5).

map = [];
m=[];
n=[];
a=[];

for i=1:nargin
   if max(size(varargin{i})) > 1
      map = varargin{i};
      m = size(map,1);
   elseif length(varargin{i}) == 1 
      if isempty(n)
         n = varargin{i};
      else
         a = varargin{i};
      end
   end
end

if isempty(map)
   map = get(gcf,'colormap');
   m = size(map,1);
end
if isempty(n)
   n = 5;
end
if isempty(a)
   a = .5;
end

for i=1:3
   map(:,i) = map(:,i).*(a*sin([0:(m-1)]'*pi*n/(m)).^2+(1-a));
end

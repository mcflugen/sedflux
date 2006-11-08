function hide_facies( varargin )

if ( nargin==1 )
   ax = varargin{1};
else
   ax = gca;
end

h = get(ax,'child');
h = h(strmatch( 'sedflux-2d-image' , get(h,'tag') ));

if ( isempty(h) )
   error( 'This is not a sedflux2d image' );
end

data = get(h,'userdata');
if ( ~isempty( data ) )
   set(h,'cdata',data);
end
set(h,'userdata',[]);


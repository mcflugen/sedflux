function map = land_water_map( arg1 , varargin )

valid_args = { 'sealevel'   , 'double' , 0 ; ...
               'zdir'       , 'char'   , 'norm' };

values = parse_varargin( valid_args , varargin );

sea_level  = values{strmatch( 'sealevel'  , {valid_args{:,1}} , 'exact' )};
z_dir      = values{strmatch( 'zdir'      , {valid_args{:,1}} , 'exact' )};

try
   axes( arg1 )
   h_ax = arg1;
   m = 256;
catch
   h_ax = [];
   m = arg1;
end

if ( ~isempty(h_ax) )

   z_lim = get(h_ax,'zlim');

   h_child = get(h_ax,'child');
   i = strmatch( 'surface' , get(h_child,'type') );
   if ( isempty( i ) )
      i = strmatch( 'image' , get(h_child,'type') )
   end
   data = get(h_child(i),'cdata');

   if ( strcmpi( z_dir , 'rev' ) )
      z_lim = -z_lim;
      data  = -data;
   end

   h = max(max(data));
   l = min(min(data));
h = z_lim(2);
l = z_lim(1);

   if ( isempty(strmatch( 'surface' , get(h_child,'type')) ) )
      z_max = h;
      z_min = l;
   else
      z_max = max( z_lim );
      z_min = min( z_lim );
   end

   if ( l<z_min ), z_min = l; end
   if ( h>z_max ), z_max = h; end

   n_land = ceil( (h-sea_level)/((h-l)/m) )
   n_water = m - n_land

   total_land  = atlas    ( ceil((z_max-sea_level)/((h-l)/m) ) );
   total_water = water_map( ceil((sea_level-z_min)/((h-l)/m) ) );

   if ( n_land<=0 )
      map = total_water( 1:m , : );
   elseif ( n_water<=0 )
      map = total_land( 1:m , : );
   else
      map = [ total_water( (end-n_water+1):end , : ) ;
              total_land( 1:n_land , : ) ];
   end
else
   map = [ water_map( floor(m/2) ) ; atlas( ceil(m/2) ) ];
end

if ( strcmpi( z_dir , 'rev' ) )
   map = map( end:-1:1 , : );
end

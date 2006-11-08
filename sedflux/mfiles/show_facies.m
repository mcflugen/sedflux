function show_facies( varargin )

valid_args = { 'color'      , 'double' , 1          ; ...
               'axis'       , 'double' , gca        ; ...
               'file'       , 'char'   , []         ; ...
               'faciesmask' , 'cell'   , []         ; ...
               'facies'     , 'char'   , 'bedload'  };

values = parse_varargin( valid_args , varargin );

color_ind   = values{strmatch( 'color'      , {valid_args{:,1}} , 'exact' )};
ax          = values{strmatch( 'axis'       , {valid_args{:,1}} , 'exact' )};
file        = values{strmatch( 'file'       , {valid_args{:,1}} , 'exact' )};
facies      = values{strmatch( 'facies'     , {valid_args{:,1}} , 'exact' )};
facies_mask = values{strmatch( 'faciesmask' , {valid_args{:,1}} , 'exact' )};

facies_no = get_facies_no_from_name( facies );

h = get(ax,'child');
h = h(strmatch( 'sedflux-2d-image' , get(h,'tag') ));

if ( isempty(h) )
   error( 'This is not a sedflux2d image' );
end

if ( isempty(get(h,'userdata')) )
   data = get(h,'cdata');
   set(h,'userdata',data);
else
   data = get(h,'userdata');
end

if ( ~isempty(facies_mask) )
   f = facies_mask;
elseif ( ~isempty( file ) )
   f = get_facies( file );
   f = f{facies_no};
else
   f = get_facies( ax );
   f = f{facies_no};
end

data(f~=0) = color_ind;
set(h,'cdata',data);

function facies_no = get_facies_no_from_name( facies_name )

facies_name = lower( facies_name );

possible_facies_names = { 'bedload'     , ...
                          'plume'       , ...
                          'debris_flow' , ...
                          'turbidite'   , ...
                          'diffused'    , ...
                          'river'       , ...
                          'wave'        };

facies_no = strmatch( facies_name , possible_facies_names );


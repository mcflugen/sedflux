function mask = make_property_mask( file , varargin )

valid_args = {
               'lim'        , 'double' , [] ; ...
               'func'       , 'function_handle' , [] };

values = parse_varargin( valid_args , varargin );

lim            = values{strmatch( 'lim'     , {valid_args{:,1}} , 'exact' )};
func           = values{strmatch( 'func'    , {valid_args{:,1}} , 'exact' )};

[data header] = read_property(file,'dim',false);

mask = logical( zeros( size(data) ) );

if ( ~isempty(lim) )
   mask( data>lim(1) & data<lim(2) ) = true;
else
   mask( func(data) ) = true;
end


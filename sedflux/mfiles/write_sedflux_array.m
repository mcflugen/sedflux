function n = write_sedflux_array( filename , data , varargin )

valid_args = { 'comment'     , 'char'    , [] ; ...
               'notranspose' , 'logical' , false };

values = parse_varargin( valid_args , varargin );

comment      = values{strmatch( 'comment'      , {valid_args{:,1}} , 'exact' )};
no_transpose = values{strmatch( 'notranspose'  , {valid_args{:,1}} , 'exact' )};

[n_rows , n_cols] = size(data);

if ( n_rows > n_cols & ~no_transpose )
   data = data';
end

[n_rows , n_cols] = size(data);

format_string = '%f';
for i=2:n_rows
   format_string = [ format_string ', %f' ];
end
format_string = [ format_string '\n' ];

fid = fopen( filename , 'w' );

n = 0;

if ( ~isempty( comment ) )
   n = n + fprintf( fid , '# %s\n' , comment );
end

n = n + fprintf( fid , '--- data ---\n' );

n = n + fprintf( fid , format_string , data );

fclose( fid );


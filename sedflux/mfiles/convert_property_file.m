function n_bytes = convert_property_file( file_in , file_out , varargin )

valid_args = { 'type'   , 'char'   , 'old' ; ...
               'water'  , 'double' , nan ; ...
               'rock'   , 'double' , nan };

values = parse_varargin( valid_args , varargin );

type      = values{strmatch( 'type'  , {valid_args{:,1}} , 'exact' )};
water_val = values{strmatch( 'water' , {valid_args{:,1}} , 'exact' )};
rock_val  = values{strmatch( 'rock'  , {valid_args{:,1}} , 'exact' )};

[fid_out,msg]= fopen( file_out , 'w' );
if ( fid_out < 0 )
   error( msg );
end

if ( strcmpi( type , 'FLAT' ) )
   [data header] = read_property( file_in , ...
                                  'dim'   , true , ...
                                  'water' , water_val , ...
                                  'rock'  , rock_val );
   fwrite( fid_out , data , 'double' );
else
   [data header] = read_property(file_in,'dim',false);
   write_old_property_header( fid_out , header );
   fwrite( fid_out , data , 'uint8' );
end

n_bytes = ftell( fid_out );

fclose( fid_out );



function write_old_property_header( fid , hdr )

fprintf( fid , '# Created by convert_property_file.\n' );

fprintf( fid , 'property : %d\n' , get_property_number( hdr.property ) );
fprintf( fid , 'n_rows : %d\n' , hdr.n_rows );
fprintf( fid , 'n_cols : %d\n' , hdr.n_y_cols );
fprintf( fid , 'cell height : %f\n' , hdr.cell_height );
fprintf( fid , 'cell width : %f\n' , hdr.dy );
fprintf( fid , 'minimum value: %f\n' , hdr.min_val );
fprintf( fid , 'maximum value: %f\n' , hdr.max_val );
fprintf( fid , 'sea level: %f\n' , hdr.sea_level );
fprintf( fid , 'co-ordinates of lower left corner : %f, %f\n' , ...
               hdr.ref_y , hdr.ref_z );
fprintf( fid , 'water value : %f\n' , hdr.water_val );
fprintf( fid , 'rock value : %f\n' , hdr.rock_val );


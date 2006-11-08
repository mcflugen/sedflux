function z = read_sedflux_grid( file )

fid = fopen( file , 'r' , 'native' );

[name , mode , byte_order] = fopen( fid );

try,
   n_x = fread( fid , 1 , 'int32' );
   z   = fread( fid , [n_x inf] , 'double' );

   if ( n_x > prod( size(z) ) )
      error( 'There was an error reading the grid file.' );
   end
catch,

   if ( strcmp( byte_order , 'ieee-le' ) )
      byte_order = 'ieee-be';
   else
      byte_order = 'ieee-le';
   end
   disp( 'The byte order of this file does not appear to be native.' );
   disp( ['Switching the byte order to the following: ' byte_order] );

   fid = fopen( file , 'r' , byte_order );
   n_x = fread( fid , 1 , 'int32' );
   z   = fread( fid , [n_x inf] , 'double' );
end

fclose(fid);


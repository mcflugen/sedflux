function [z,t] = read_sedflux_grid_sequence( file )

fid = fopen( file , 'r' , 'native' );

[name , mode , byte_order] = fopen( fid );

try,

   n_x = fread( fid , 1 , 'int32' );
   n_y = fread( fid , 1 , 'int32' );

   z = fread( fid , [(n_x*n_y+1) inf] , 'double' );

   t = z(1,:);
   z(1,:) = [];

   z = reshape( z , n_x , n_y , size(t,2) );

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
   n_y = fread( fid , 1 , 'int32' );

   z = fread( fid , [(n_x*n_y+1) inf] , 'double' );

   t = z(1,:);
   z(1,:) = [];

   z = reshape( z , n_x , n_y , size(t,2) );

end


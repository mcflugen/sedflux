function n = write_sedflux_grid_sequence( fid , z , t )

t = reshape( t , 1 , length(t) );

if ( size(z,3) ~= length(t) )
   error( [ 'The number of time values in parameter, ''t'' must match the\n' ...
            'number of grids in the sequence.'] );
end


n = 0;

n = n + fwrite( fid , size(z,1) , 'int32' );
n = n + fwrite( fid , size(z,2) , 'int32' );

z = reshape( z , size(z,1)*size(z,2) , length(t) );

n = n + fwrite( fid , [ t ; z ] , 'double' );



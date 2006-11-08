function n = write_sedflux_grid( fid , z )

n = 0;
n = n + fwrite( fid , size(z,1) , 'int32' );
n = n + fwrite( fid , z         , 'double' );


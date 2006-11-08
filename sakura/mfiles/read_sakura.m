function [x,z,y,data]=read_sakura( filename , n_grains )

fid = fopen( filename , 'r' );

n = fread( fid , 1 , 'int' )-1;
data = fread( fid , [n 3] , 'double' );
x = data( : , 1 );
z = data( : , 2 );
y = data( : , 3 );

data = fread( fid , [n*5 inf] , 'double' );
data = reshape( data , n , 5 , prod(size(data))/n/5 );

%x  = squeeze(data( : , 1 , : ));
u  = squeeze(data( : , 2 , : ));
h  = squeeze(data( : , 3 , : ));
c  = squeeze(data( : , 4 , : ));
dz = squeeze(data( : , 5 , : ));

subplot( 4,1,1 )
plotyy( x , u , x , z )
subplot( 4,1,2 )
plotyy( x , h , x , z )
subplot( 4,1,3 )
plotyy( x , c , x , z )
subplot( 4,1,4 )
plotyy( x , dz , x , z )

fclose( fid );


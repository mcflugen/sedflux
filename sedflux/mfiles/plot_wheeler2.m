function plot_wheeler( filename , river_mouth_file )

if ( nargin>1 )
   river_mouth = 1;
else
   river_mouth = 0;
end

if ( river_mouth )
   [rm_data,rm_loc,rm_t,rm_hdr] = read_measuring_station( river_mouth_file );
end

fid = fopen( filename , 'r' );
z = fread( fid , [1000 1000] , 'double' );

d = diff( z );
d(d<0) = -10;
d(d>0) = 10;

%map = [ 1 1 1 ; 1 0 0 ; 0 1 0 ];
%colormap( map );

imagesc( loc(:,1)/1000 , t/1000 , d );

set(gca,'ydir','norm')

xlabel( 'Distance (km)' )
ylabel( 'Time (kyr)' )

if ( river_mouth==1 )
   hold on
   plot( rm_loc/1000 , rm_t/1000 , 'b' , 'linewidth' , 3 )
   hold off
end


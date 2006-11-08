function plot_wheeler( filename , river_mouth_file )

if ( nargin>1 )
   river_mouth = 1;
else
   river_mouth = 0;
end

[data,loc,t,hdr]             = read_measuring_station( filename );
if ( river_mouth )
   [rm_data,rm_loc,rm_t,rm_hdr] = read_measuring_station( river_mouth_file );
end


change = diff(data,1,2);

dep = change>.1;
ero = change<0;

%map = [ 1 1 1 ; 1 0 0 ; 0 1 0 ];
%colormap( map );

d = 1+(zeros(size(data)));
d(dep) = 2;
d(ero) = 0;

imagesc( loc(:,1)/1000 , t , d' );
orig_gca = gca;

h_cb = colorbar( 'horiz' )
axes(h_cb)
xlabel( 'Erosion / Deposition' )

axes( orig_gca )

set(gca,'ydir','norm')

xlabel( 'Distance (km)' )
ylabel( 'Time (years)' )

colormap( wheeler )

if ( river_mouth==1 )
   hold on
   plot( rm_loc/1000 , rm_t , 'r' , 'linewidth' , 1 )
   hold off
end


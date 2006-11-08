function plot_measuring_station_location( filename )
% PLOT_MEASURING_STATION_LOCATION   Plot the locations of measuring stations.
%
% PLOT_MEASURING_STATION_LOCATION( filename )

[data,x,t,hdr] = read_measuring_station( filename );

plot( x(1,:) , t )
xlabel( 'distance (m)' );
ylabel( 'time (years)' );


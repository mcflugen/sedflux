function plot_wheeler( river_mouth_file )

[rm_data,rm_loc,rm_t,rm_hdr] = read_measuring_station( river_mouth_file );

plot( rm_loc/1000 , rm_t , 'r' , 'linewidth' , 1 )


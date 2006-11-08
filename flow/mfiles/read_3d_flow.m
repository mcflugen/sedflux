function u = read_3d_flow( infile )

fid = fopen( infile );
u = fread( fid , [17^3 inf] , 'double' )';
fclose(fid);


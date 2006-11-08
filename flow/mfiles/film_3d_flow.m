function film_3d_flow( infile , outfile )

fid=fopen( infile );
c=fread(fid,[33^3 inf],'double' )';
fclose(fid);
for i=1:size(c,1)
    x = 1:33;
y = x;
z = x;
[x,y,z]=meshgrid(x,y,z);


    c_3d = reshape( c(i,:) , 33 , 33 , 33 );
    c_1 = subvolume( c_3d , [17 33 17 33 nan nan] );
    x = subvolume( x , [17 33 17 33 nan nan] );
    y = subvolume( y , [17 33 17 33 nan nan] );
    z = subvolume( z , [17 33 17 33 nan nan] );
%    c_1(end,end,end) = 100;
    
    p = patch( isosurface(x,y,z,c_1,0) , 'facecolor' , 'red' , 'edgecolor' , 'none' );
    p2 = patch(isocaps(x,y,z,c_1,0) , 'facecolor','interp','edgecolor','none');
    view(3)
    xlim([1 33])
    ylim([1 33])
    zlim([1 33])
    box on
    camlight(0,35);
    lighting gouraud;
    isonormals(x,y,z,c_1,p);

    m(i) = getframe(gcf);
    clf
end

movie2avi( m , outfile , 'fps' , 2 , 'quality' , 100 , 'compression' , 'none' )

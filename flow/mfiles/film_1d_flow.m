function film_1d_flow( infile , outfile )

u = dlmread( infile );
t = u(:,1);
u(:,1) = [];

z = linspace( 0 , 1 , size(u,2) );

for i=1:length(t)
   plot( u(i,:) , z );
   xlim([0 1])
   xlabel( 'excess porewater pressure' )
   ylabel( 'depth' )
   m(i) = getframe(gca);
end

movie2avi( m , outfile , 'fps' , 5 , 'quality' , 100 , 'compression' , 'none' )

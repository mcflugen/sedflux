function u=plot_2d_flow( infile )

u = dlmread( infile );
t = u(:,1);
u(:,1) = [];

[u,t]=read_2d_flow( 'outfile' );

z = linspace( 100 , 0 , size(u,2) );

for i=2:length(t)
   p = reshape( u(i,:) , 33 , 33 );
   imagesc( z , z , p , [0 1]);
   xlabel( 'excess porewater pressure' )
   ylabel( 'depth' )
   m(i-1)=getframe(gca);
end

hold off
function u=plot_1d_flow( infile )

u = dlmread( infile );
t = u(:,1);
u(:,1) = [];

z = linspace( 0 , 1 , size(u,2) );

for i=2:length(t)
   plot( u(i,:) , z , 'r' ); hold on
   xlabel( 'excess porewater pressure' )
   ylabel( 'depth' )
end

hold off
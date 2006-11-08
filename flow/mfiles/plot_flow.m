function plot_flow( file , dz )

u = dlmread( file , ',' );

t = u(:,1);
u = u(:,2:end);
z = [1:size(u,2)]*dz;
u(u==0) = nan;

[z,t] = meshgrid(z,t);

surf( z , t , u )

xlabel( 'depth (m)' );
ylabel( 'time (min)' );
zlabel( 'excess pore-water pressure' );

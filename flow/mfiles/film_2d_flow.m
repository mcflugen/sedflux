function film_2d_flow( infile , outfile )

u = dlmread( infile , ',' );
t = u(:,1);
u(:,1) = [];
x = linspace( 0 , 1 , 17 );
[x,y] = meshgrid( x , x );

for i=1:length(t)
   u_2d = reshape( u(i,:) , 17 , 17 );
%   mesh( x , y , u_2d );
%   zlim([0 1]);
%   view([-135 34])
   imagesc( u_2d , [0 1] );
   m(i) = getframe(gca);
end

movie2avi( m , outfile , 'fps' , 2 , 'quality' , 100 , 'compression' , 'none' )
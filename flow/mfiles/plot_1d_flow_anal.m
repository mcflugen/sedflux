function u=plot_1d_flow_anal( z , t , k , m_v )

t = reshape( t , 1 , length(t) );
z = reshape( z , 1 , length(z) );

gamma_w = 1;
eps = 1e-5;
err = 2*eps;
h   = 2*abs(z(1)-z(end));
[z,t] = meshgrid( z , t );
d   = h/2;
c_v = k/gamma_w/m_v;
t_v = c_v*t/d^2;
u_0 = 1;

u = zeros( size(z) );
n=0;
while err>eps
%for n=1:500
   u_old = u;
   u = u + 4/(2*n+1)/pi*sin( (2*n+1)*pi*z/h ).*exp( -(2*n+1)^2*pi^2*t_v/4 );
   err   = norm( u-u_old );
   n = n+1;
end

u = u*u_0;
mesh( z(:,end:-1:1) , t , u );
xlabel( 'z' );
ylabel( 'time' );
zlabel( 'excess porewater pressure' );
view([-38+180 30])
%plot( u , z(end:-1:1) );

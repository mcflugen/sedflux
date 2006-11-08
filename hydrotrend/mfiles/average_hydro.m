function m = average_hydro( x , n )

v_exp = .4;
w_exp = .1;
v_coef = .035;
w_coef = 100;

n_grain = size(x,1)-4;
n_rec   = floor(size(x{1},2)/n);

if ( n_rec*n ~= size(x{1},2) )
   disp( ['Using only first ' num2str(n_rec*n) ' data values'] );
   for i=1:length(x)
      x{i} = x{i}(1:(n_rec*n));
   end
end

v  = x{1};
w  = x{2};
d  = x{3};
qb = x{4};
q  = v.*w.*d;
for i=1:n_grain
   qs{i} = x{i+4}.*q;
end

q  = mean( reshape( q  , [n n_rec] ) , 1 );
%v  = mean( reshape( v  , [n n_rec] ) , 1 );
%d  = mean( reshape( d  , [n n_rec] ) , 1 );
qb = mean( reshape( qb , [n n_rec] ) , 1 );
%w  = q./(v.*d);

v = v_coef*q.^v_exp;
w = w_coef*q.^w_exp;
d = q./(v.*w);

m{1} = v;
m{2} = w;
m{3} = d;
m{4} = qb;

for i=1:n_grain
   m{i+4} = mean( reshape( qs{i} , [n n_rec] ) , 1 )./q;
end

m = m';

disp( 'Checking year 1.' )
disp( ['Average velocity ' num2str(v(1))] )
disp( ['Average width ' num2str(w(1))] )
disp( ['Average depth ' num2str(d(1))] )
disp( ['Average concentration ' num2str(m{5}(1))] )
disp( ['Total sediment ' num2str(v(1)*w(1)*d(1)*m{5}(1))] )

qs = x{1}.*x{2}.*x{3}.*x{5};
disp( ['Total sediment ' num2str(sum(qs(1:365)))] )


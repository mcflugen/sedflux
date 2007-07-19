function x = combine_hydro( file_filter , varargin )

   valid_args = { 'format' , 'char' , 'native' };

   values = parse_varargin( valid_args , varargin );

   format = values{ strmatch( 'format' , {valid_args{:,1}} , 'exact' )};

filename = [];
if ( iscell( file_filter ) )
   for i=1:length(file_filter)
      file = dir(file_filter{i});
      for n=1:length(file)
         filename{end+1} = file(n).name;
      end
   end
%   filename = sort(filename);
else
   file = dir(file_filter);

   for n=1:length(file)
      filename{n} = file(n).name;
   end
   filename = sort(filename);
end

disp( ['Found ' num2str(length(filename)) ' files'] );

n_grain = 4;
q_tot  = 0;
qb_tot = 0;
qs_tot = num2cell( zeros(n_grain,1) );
for i=1:length( filename )
   disp( ['Reading file: ' filename{i}]);
   data  = read_hydro( filename{i} , 'format' , format );
   
   [v,w,d,qb] = deal( data{1:4} );

   q = v.*w.*d;

   for n=1:n_grain
      qs_tot{n} = qs_tot{n} + data{n+4}.*q;
   end

   q_tot  = q_tot + q;
   qb_tot = qb_tot + qb;

end

v_exp  = .1;
w_exp  = .5;
v_coef = .26;
w_coef = 9.7;

v_tot = v_coef*q_tot.^v_exp;
w_tot = w_coef*q_tot.^w_exp;
d_tot = q_tot./(v_tot.*w_tot);

cs_tot = num2cell( zeros(n_grain,1) );
for n=1:n_grain
   cs_tot{n} = qs_tot{n}./q_tot;
end

x = { v_tot , w_tot , d_tot , qb_tot , cs_tot{:} }';



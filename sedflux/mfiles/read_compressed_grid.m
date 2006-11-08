function g = read_compressed_grid( fid , varargin )

valid_args = { 'lim' , 'double' , [] };

values = parse_varargin( valid_args , varargin );

lim = values{strmatch( 'lim'  , {valid_args{:,1}} , 'exact' )};

if ( ~isempty(lim) )

disp( ['Reading limits ' num2str( lim )] );

   goto_element( fid , lim(1) );
   g = read_n_elements( fid , lim(2) );

else

   start = ftell(fid);
disp( 'determine the size of the grid' );   
   %%%
   %%% First determine the size of the grid.
   %%%
   tot = 0;
   s=fread( fid , 1 , 'int' );
   while ( ~isempty( s ) )
      n = fread( fid , 1 , 'int' );
      tot = tot + n*s;
      fseek( fid , s , 'cof' );
   
      s=fread( fid , 1 , 'int' );
   end
   
   %%%
   %%% Allocate the memory for the grid.
   %%%
disp( ['allocate memory: ' num2str(tot) ' bytes'] );
   g = ones( 1 , tot/8 );
   
   %%%
   %%% Now read in the file and read the data into the (allocated) grid.
   %%%
   fseek( fid , start , 'bof' );
   
disp( 'read the data' );
   i = 1;
   s=fread( fid , 1 , 'int' );
   while ( ~isempty( s ) )
   
      n = fread( fid , 1 , 'int' );
   
      if ( n==1 )
         n = s/8;
         g(i:(i+n-1)) = fread( fid , [1 n] , 'double' );
      else
         val = fread( fid , 1   , 'double' );
         g(i:(i+n-1)) = val*ones(1,n);
      end
   
      i = i+n;
      s=fread( fid , 1 , 'int' );
   
   end

end

function goto_element( fid , element )

tot = 0;
s   = fread( fid , 1 , 'int' );
while ( ~isempty( s ) & tot < element )
   n   = fread( fid , 1 , 'int' );
   if ( n==1 )
      tot = tot + s/8;
      fseek(  fid , s , 'cof' );
   else
      tot = tot + n;
      fseek(  fid , 8 , 'cof' );
   end

   s   = fread( fid , 1 , 'int' );
end

if ( ~isempty( s ) )
   fseek( fid , -4 , 'cof' );
end

function g = read_n_elements( fid , n_elements )

% s is the size in bytes of the record.
% n is the number of elements in the record.
% if n is greater than one, the next value is repeated n times.
% if n is one, an array of unique values follows (of length s bytes).
% Currently each of these values is a double (size of 8 bytes).

tot = 0;
i   = 1;
s   = fread( fid , 1 , 'int' );
while ( ~isempty( s ) & tot<n_elements )

   n = fread( fid , 1 , 'int' );

   if ( n==1 )
      n            = s/8;
      g(i:(i+n-1)) = fread( fid , [1 n] , 'double' );
   else
      val          = fread( fid , 1   , 'double' );
      g(i:(i+n-1)) = val*ones(1,n);
   end

   tot = tot + n;

   i = i+n;
   s = fread( fid , 1 , 'int' );

end


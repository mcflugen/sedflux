function write_hydro( file , data , n_seasons )

fid = fopen( file , 'w' );

data = cell2mat(data);

comment   = date;
n_grains  = size(data,1) - 4
n_samples = size(data,2)

n_bytes =           fwrite( fid , length(comment) , 'int'  );
n_bytes = n_bytes + fwrite( fid , comment         , 'char' );

n_bytes = n_bytes + fwrite( fid , n_grains  , 'int' );
n_bytes = n_bytes + fwrite( fid , n_seasons , 'int' );
n_bytes = n_bytes + fwrite( fid , n_samples , 'int' );

n_bytes = n_bytes + fwrite( fid , data , 'float' );

fclose( fid );


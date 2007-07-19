function varargout=hydro_info( file , varargin )
% HYDRO_INFO   Get information about a hydrotrend output file.
%
% SEE ALSO PLOT_HYDRO, READ_HYDRO
%

   valid_args = { 'format' , 'char' , 'native' };

   values = parse_varargin( valid_args , varargin );

   format = values{ strmatch( 'format' , {valid_args{:,1}} , 'exact' )};

   fid = fopen( file , 'r' , format );
   if ( fid < 0 )
      error( [ 'Can not open file ' file ] );
   end
   
   comment_len = fread(fid ,   1               , 'int'         );

   if ( comment_len < 0 | comment_len > 2048 )
      error( 'Trouble reading comment.  Possible incorrect byte order?' );
   end
   comment     = fread(fid , [ 1 comment_len ] , 'uint8=>char' );

   n_grains  = fread( fid , 1 , 'int' );
   n_seasons = fread( fid , 1 , 'int' );
   n_samples = fread( fid , 1 , 'int' );
   
   n_years = n_samples / n_seasons;
   
   fclose(fid);
   
   if nargout > 0
      varargout{1}=n_seasons;
      if nargout > 1
         varargout{2}=n_years;
         if nargout > 2
            varargout{3}=n_grains;
         end
      end
   else
      disp( ['File                  : ' file               ] );
      disp( ['Number of seasons     : ' num2str(n_seasons) ] );
      disp( ['Number of grain sizes : ' num2str(n_grains ) ] );
      disp( ['Number of years       : ' num2str(n_years  ) ] );
      disp( ['Comment               : ' comment            ] );
   end
   

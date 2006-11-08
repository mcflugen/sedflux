function varargout=hydro_info(varargin)
% HYDRO_INFO   Get information about a hydrotrend output file.
%
% SEE ALSO PLOT_HYDRO, READ_HYDRO
%

   file_name='river.in';
   if nargin==1
      file_name = varargin{1};
   end
   
   fid=fopen(file_name);
   if ( fid < 0 )
      error_str=sprintf('Can not open file %s',file_name);
      error(error_str);
   end
   
   
   commentBytes = fread(fid,1,'int');
   comment = fread(fid,commentBytes,'uint8=>char');
   
   nGrains = fread(fid,1,'int');
   nSeasons = fread(fid,1,'int');
   nSamples = fread(fid,1,'int');
   
   nYears=nSamples/nSeasons;
   
   fclose(fid);
   
   if nargout > 0
      varargout{1}=nSeasons;
      if nargout > 1
         varargout{2}=nYears;
         if nargout > 2
            varargout{3}=nGrains;
         end
      end
   else
      fprintf(1,'File                  : %s\n',file_name);
      fprintf(1,'Number of seasons     : %d\n',nSeasons);
      fprintf(1,'Number of grain sizes : %d\n',nGrains);   
      fprintf(1,'Number of years       : %d\n',nYears);   
      fprintf(1,'Comment               : %s\n',comment);   
   end
   

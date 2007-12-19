function [data, varargout] = read_hydro(file_name,varargin)
% READ_HYDRO   Read a hydrotrend output file.
%
% DATA = READ_HYDRO( FILENAME ) read the data from a hydrotrend file.  the 
% are stored in a series of cells as such:
%    DATA{1}     - River velocity (m/s)
%    DATA{2}     - River width (m)
%    DATA{3}     - River depth (m)
%    DATA{4}     - Bed Load flux (kg/s)
%    DATA{5:end} - Suspended load Concentration (kg/m^3)
% the number of cells will depend on the number of grain sizes that are given
% in the hydrotrend file.
%
% DATA = READ_HYDRO( FILENAME , LIM ) read only the data from FILENAME
% contained within LIM.  if LIM is a scalar, only the first LIM records are
% read.  alternatively, LIM can be a two element vector that gives the first
% and last record to read.
%
% DATA = READ_HYDRO( FILENAME , OUTTYPE ) if a string argument is passed, it
% is interpreted as a specifier that gives the type of output.  valid values
% for OUTTYPE are:
%    'qs'  - total suspended sediment load (kg/s)
%    'qb'  - total bed load (kg/s)
%    'cs'  - total suspended sediment concentration (kg/m^3)
%    'q'   - water discharge (m^3/s)
%    'raw' - raw data from the hydrotrend file (this is the default)
%
% [DATA,NSEASONS] = READ_HYDRO( FILENAME ) a second output argument can be
% specified to determine the number of seasons that were modeled in the
% hydrotrend file.
%
% SEE ALSO PLOT_HYDRO, HYDRO_INFO
%
   
   valid_args = { 'format'  , 'char'   , 'native' ; ...
                  'records' , 'double' , []       ; ...
                  'output'  , 'char'   , 'raw'   };

   values = parse_varargin( valid_args , varargin );

   format  = values{ strmatch( 'format'  , {valid_args{:,1}} , 'exact' )}
   records = values{ strmatch( 'records' , {valid_args{:,1}} , 'exact' )}
   output  = values{ strmatch( 'output'  , {valid_args{:,1}} , 'exact' )}

   type = strmatch( output , { 'qs' , 'qb' , 'q' , 'cs' , 'raw' } , 'exact' );
   if ( isempty(type) )
      error( ['Unrecognized output type -- ' output ]);
   end

   [n_seasons maxYear n_grains]=hydro_info( file_name , 'format' , format )

   if ( isempty(records) )
      start_record = 1;
      n_records    = maxYear*n_seasons;
   elseif ( isscalar(records) )
      start_record = 1;
      n_records    = records;
   elseif ( length(records) == 2 )
      start_record = records(1);
      n_records    = records(2);
   else
      error( 'Invalid value for RECORDS parameter' );
   end

   top_record = n_seasons*maxYear;
   if ( start_record + n_records-1 > top_record )
      n_records=top_record-start_record+1;
   end
   
   fid = fopen( file_name , 'r' , format );
   if ( fid < 0 )
      error( ['Can not open file: ' file_name ] );
      data = -1;
      for i=1:nargout
         varargout{i}=-1;
      end
   end
   
   skip_header(fid);

%%%   
%%% Skip the first start_record records.
%%%
   skip_river_record( fid , n_grains , start_record-1 );
   
   values_per_record = 4+n_grains;

   data = fread(fid,[values_per_record n_records],'float');
   
   fclose(fid);

   if ( strcmp( output , 'qs' ) )
      data = data(1,:).*data(2,:).*data(3,:).*(sum(data(5:end,:),1));
   elseif ( strcmp( output , 'qb' ) )
      data = data(4,:);
   elseif ( strcmp( output , 'q' ) )
      data = data(1,:).*data(2,:).*data(3,:);
   elseif ( strcmp( output , 'cs' ) )
      data = sum(data(5:end,:));
   else
      data=num2cell(data,2);
   end
   
   if nargout == 2
      varargout{1} = [start_record:(start_record+n_records-1)]/n_seasons;
   end

function skip_header( fid )

   comment_bytes = fread( fid , 1 , 'int' );
   fread( fid , comment_bytes , 'char' );
   n_grains = fread( fid , 3 , 'int' );

function skip_river_record( fid , n_grains , skip_records )

   bytes_per_record = (4+n_grains)*4;
   fseek(fid,bytes_per_record*skip_records,'cof');



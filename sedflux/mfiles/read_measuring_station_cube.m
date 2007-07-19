%% Read data from a sedflux measuring station file
%
% The measuring station data is read into a 3D matrix.  The dimensions
% of the matrix will be (n_x,n_y,n_t).  Where n_x, and n_y are the
% number of nodes in the x and y directions, respectively and n_t
% is the number of sample times.
%
% The locations of the stations is given as a two row matrix.  The first
% row indicates the x-values and the second row the y-values.
%
% Optional paramter/value pairs:
%   - timeslice : Read only data sampled at this time.  The value
%                 is an index (starting at 1) to the sample. [int]
%   - diff      : Indicate if a time derivative of the data be taken. [boolean]
%
% \param filename   Name of the measuring station file
% \param varargin   Optional parameter/value pairs
%
% \returns [data,loc,time,hdr] Measuring station data, location, sample time,
%          and header information.
%
function [data,loc,time,hdr] = read_measuring_station_cube( filename  , varargin )
% READ_MEASURING_STATION   Read the data from a sedflux measuring station file.
%
% READ_MEASURING_STATION( filename )
%
% SEE ALSO
%

valid_args = { 'timeslice'  , 'double' , [] ; ...
               'diff'       , 'logical' , false } ;

values = parse_varargin( valid_args , varargin );

time_slice      = values{strmatch( 'timeslice' , {valid_args{:,1}} , 'exact' )};
time_derivative = values{strmatch( 'diff'      , {valid_args{:,1}} , 'exact' )};

%
% read the header information.  close the file so that we can reopen it
% with the correct byte order (which we get from the first reading of the
% header).
%

fid   = fopen( filename , 'r' );
hdr   = read_tripod_header( fid );
fclose(fid);

%
% read the byte order that the data were written with.  reopen the file
% with the specified byte order.
%

if ( hdr.byte_order == 4321 )
   fid = fopen( filename , 'r' , 'ieee-be' );
else
   fid = fopen( filename , 'r' , 'ieee-le' );
end
hdr   = read_tripod_header( fid );

%
% read the number of elements in each record.  then read all of the records.
%
n     = fread( fid , 1 , 'int32' );

%
% Determine that number of records in the file.
%
here      = ftell( fid );
fseek( fid , 0 , 'eof' );
there     = ftell( fid );
n_records = (there-here)/(n*8);

fseek( fid , here , 'bof' );

if ( isempty( time_slice ) )

   a = fread( fid , [n n_records] , 'double' );

%   a = fread( fid , [n 50] , 'double' );
else

   if ( time_derivative )
      records_to_read = 2;
   else
      records_to_read = 1;
   end

   if ( isinf( time_slice ) | time_slice==n_records )
      fseek( fid , -n*8*records_to_read , 'eof' );
   else
      fseek( fid , n*(time_slice-1)*8 , 'cof' );
   end
   a = fread( fid , [n records_to_read] , 'double' );

end

n = (n-1)/3;

time = a( 1           , : );
loc  = a( 2:(2*n+1)   , : );
data = a( (2*n+2):end , : );

if ( hdr.n_tripods == hdr.n_x_cols*hdr.n_y_cols )
   data = reshape( data , [hdr.n_y_cols hdr.n_x_cols size(data,2)]);
end

loc = reshape( loc , 2 , hdr.n_x_cols*hdr.n_y_cols , [] );
a = [];

if ( time_derivative )
   data = diff( data , 1 , 3 );
   loc(:,:,end) = [];
   time(end) = [];
end

fclose(fid);



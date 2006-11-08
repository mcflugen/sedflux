function [data,loc,time,hdr] = read_measuring_station( filename )
% READ_MEASURING_STATION   Read the data from a sedflux measuring station file.
%
% READ_MEASURING_STATION( filename )
%
% SEE ALSO
%

%%%
%%% read the header information.  close the file so that we can reopen it
%%% with the correct byte order (which we get from the first reading of the
%%% header).
%%%

fid   = fopen( filename , 'r' );
hdr   = read_measuring_station_header( fid )
fclose(fid);

%%%
%%% read the byte order that the data were written with.  reopen the file
%%% with the specified byte order.
%%%

if ( hdr.byte_order == 4321 )
   fid = fopen( filename , 'r' , 'ieee-be' )
else
   fid = fopen( filename , 'r' , 'ieee-le' )
end

%%%
%%% read the number of elements in each record.  then read all of the records.
%%%

try
   hdr   = read_measuring_station_header( fid )
   n     = fread( fid , 1 , 'int64' )
   a     = fread( fid , [n inf] , 'double' );
catch
   fseek( fid , 0 , 'bof' );
   hdr   = read_measuring_station_header( fid )
   n     = fread( fid , 1 , 'int32' )
   a     = fread( fid , [n inf] , 'double' );
end


time  = a(1,:);
loc   = a(2:((n+1)/2),:);
data  = a(((n+3)/2):end,:);

a = [];

fclose(fid);


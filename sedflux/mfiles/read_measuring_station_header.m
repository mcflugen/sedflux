function hdr = read_property_header(fid)
% READ_MEASURING_STATION_HEADER   Read a sedflux measuring station header.
%
% HEADER = READ_MEASURING_STATION_HEADER( FILENAME ) Reads header
%  information from the sedflux measuring station file, FILENAME
%
% SEE ALSO
%

   fgets(fid);
   [label,entry]=strtok(fgets(fid),':');
   hdr.byte_order = sscanf(entry,':%d',1);
   [label,entry]=strtok(fgets(fid),':');
%%%   hdr.parameter = sscanf(entry,':%s',1);
   hdr.parameter = entry(2:end);
   [label,entry]=strtok(fgets(fid),':');
%%%   hdr.origin = sscanf(entry,':%s',1);
   hdr.origin = entry(2:end);
   [label,entry]=strtok(fgets(fid),':');
%%%   hdr.locs = sscanf(entry,':%s',1);
   if ( ~isempty( strfind(upper(entry),'ALL LOCATIONS') ) )
      hdr.locs = 'all';
   else
      hdr.locs = str2num(entry(2:end));
   end


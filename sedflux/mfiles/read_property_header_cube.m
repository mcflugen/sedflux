function hdr = read_property_header_cube(fid)
% READ_PROPERTY_HEAEDER   Read a sedflux header file.
%
% HEADER = READ_PROPERTY_HEADER( filename ) Reads header
%  information from the sedflux property file, filename
%
% SEE ALSO
%

   fgets(fid);
   [label,entry]=strtok(fgets(fid),':');
   hdr.property = sscanf(entry,':%d',1);

   [label,entry]=strtok(fgets(fid),':');
   hdr.n_rows = sscanf(entry,':%d',1);

   [label,entry]=strtok(fgets(fid),':');
   hdr.n_x_cols = sscanf(entry,':%d',1);

   [label,entry]=strtok(fgets(fid),':');
   hdr.n_y_cols = sscanf(entry,':%d',1);

   [label,entry]=strtok(fgets(fid),':');
   hdr.cell_height = sscanf(entry,':%f',1);

   [label,entry]=strtok(fgets(fid),':');
   hdr.cell_x_width = sscanf(entry,':%f',1);

   [label,entry]=strtok(fgets(fid),':');
   hdr.cell_y_width = sscanf(entry,':%f',1);

   [label,entry]=strtok(fgets(fid),':');
   hdr.min_val = sscanf(entry,':%f',1);

   [label,entry]=strtok(fgets(fid),':');
   hdr.max_val = sscanf(entry,':%f',1);

   [label,entry]=strtok(fgets(fid),':');
   hdr.sea_level= sscanf(entry,':%f',1);

   [label,entry]=strtok(fgets(fid),':');
   hdr.ref = sscanf(entry,':%f,%f,%f',3);

   [label,entry]=strtok(fgets(fid),':');
   hdr.water_val = sscanf(entry,':%f',1);

   [label,entry]=strtok(fgets(fid),':');
   hdr.rock_val = sscanf(entry,':%f',1);


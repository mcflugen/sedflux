%% Read header information from a sedflux tripod (measuring station) file
%
% \param fid   File id of the tripod file
%
% \returns A structure that contains the header information
%
function hdr = read_tripod_header( fid )
% READ_PROPERTY_HEAEDER   Read a sedflux header file.
%
% HEADER = READ_PROPERTY_HEADER( filename ) Reads header
%  information from the sedflux property file, filename
%
% SEE ALSO
%

   h = parse_sedflux_header( fid );

   hdr.property    = h(strmatch( 'Property'            , { h.l } , 'exact' )).v;
   hdr.n_tripods   = h(strmatch( 'Number of tripods'   , { h.l } , 'exact' )).v;
   hdr.n_x_cols    = h(strmatch( 'Number of x-columns' , { h.l } , 'exact' )).v;
   hdr.n_y_cols    = h(strmatch( 'Number of y-columns' , { h.l } , 'exact' )).v;
   hdr.no_data     = h(strmatch( 'No data value'       , { h.l } , 'exact' )).v;
   hdr.byte_order  = h(strmatch( 'Byte order' , { h.l } , 'exact' )).v;
   hdr.data_type   = h(strmatch( 'Data type' , { h.l } , 'exact' )).v;
   hdr.origin      = h(strmatch( 'Origin' , { h.l } , 'exact' )).v;

   hdr.n_tripods   = str2double( hdr.n_tripods );
   hdr.n_x_cols    = str2double( hdr.n_x_cols );
   hdr.n_y_cols    = str2double( hdr.n_y_cols );
   hdr.no_data     = str2double( hdr.no_data );
   hdr.byte_order  = str2double( hdr.byte_order );


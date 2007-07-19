%% Read header information from a sedflux property file
%
% \param fid   File id of the sedflux property file
%
% \return A structure that contains the header information
%
function hdr = read_property_header( fid )
% READ_PROPERTY_HEAEDER   Read a sedflux header file.
%
% HEADER = READ_PROPERTY_HEADER( filename ) Reads header
%  information from the sedflux property file, filename
%
% SEE ALSO
%

   h = parse_sedflux_header( fid );

   hdr.property    = h(strmatch( 'Property'            , { h.l } , 'exact' )).v;
   hdr.n_rows      = h(strmatch( 'Number of rows'      , { h.l } , 'exact' )).v;
   hdr.n_x_cols    = h(strmatch( 'Number of x-columns' , { h.l } , 'exact' )).v;
   hdr.n_y_cols    = h(strmatch( 'Number of y-columns' , { h.l } , 'exact' )).v;
   hdr.cell_height = h(strmatch( 'dz' , { h.l } , 'exact' )).v;
   hdr.dy          = h(strmatch( 'dy' , { h.l } , 'exact' )).v;
   hdr.dx          = h(strmatch( 'dx' , { h.l } , 'exact' )).v;
   hdr.sea_level   = h(strmatch( 'Sea level' , { h.l } , 'exact' )).v;
   hdr.water_val   = h(strmatch( 'Water value' , { h.l } , 'exact' )).v;
   hdr.rock_val    = h(strmatch( 'Rock value' , { h.l } , 'exact' )).v;
   hdr.byte_order  = h(strmatch( 'Byte order' , { h.l } , 'exact' )).v;
   hdr.data_type   = h(strmatch( 'Data type' , { h.l } , 'exact' )).v;
   hdr.ref_y       = h(strmatch( 'West-side coordinate' , {h.l} , 'exact' )).v;
   hdr.ref_z       = h(strmatch( 'Bottom-side coordinate' , {h.l} , 'exact' )).v;
   hdr.ref_x       = h(strmatch( 'North-side coordinate' , {h.l} , 'exact' )).v;

   hdr.n_rows      = str2double( hdr.n_rows );
   hdr.n_x_cols    = str2double( hdr.n_x_cols );
   hdr.n_y_cols    = str2double( hdr.n_y_cols );
   hdr.cell_height = str2double( hdr.cell_height );
   hdr.dy          = str2double( hdr.dy );
   hdr.dx          = str2double( hdr.dx );
   hdr.sea_level   = str2double( hdr.sea_level );
   hdr.water_val   = str2double( hdr.water_val );
   hdr.rock_val    = str2double( hdr.rock_val );
   hdr.byte_order  = str2double( hdr.byte_order );
   hdr.ref_x       = str2double( hdr.ref_x );
   hdr.ref_y       = str2double( hdr.ref_y );
   hdr.ref_z       = str2double( hdr.ref_z );


function [data, hdr] = read_property( file_name , varargin )
% READ_PROPERTY   Read a sedflux property file.
%
% DATA = READ_PROPERTY( filename ) Reads the data from the 
%  sedflux property file 'FILE NAME'
%
% SEE ALSO
%

valid_args = { 'skip'   , 'double'          , 0 ; ...
               'dim'    , 'logical'         , true ; ...
               'format' , 'char'            , [] ; ...
               'cols'   , 'double'          , [] ; ...
               'water'  , 'double'          , [] ; ...
               'rock'   , 'double'          , [] ; ...
               'clim'   , 'double'          , [] ; ...
               'mask'   , 'logical'         , false ; ...
               'func'   , 'function_handle' , [] };

values = parse_varargin( valid_args , varargin );

skip          = values{strmatch( 'skip'   , {valid_args{:,1}} , 'exact' )};
dim           = values{strmatch( 'dim'    , {valid_args{:,1}} , 'exact' )};
format        = values{strmatch( 'format' , {valid_args{:,1}} , 'exact' )};
col_no        = values{strmatch( 'cols'   , {valid_args{:,1}} , 'exact' )};
new_water_val = values{strmatch( 'water'  , {valid_args{:,1}} , 'exact' )};
new_rock_val  = values{strmatch( 'rock'   , {valid_args{:,1}} , 'exact' )};
data_lim      = values{strmatch( 'clim'   , {valid_args{:,1}} , 'exact' )};
mask          = values{strmatch( 'mask'   , {valid_args{:,1}} , 'exact' )};
func          = values{strmatch( 'func'   , {valid_args{:,1}} , 'exact' )};

% For the DEC/ALPHA use 'ieee-le.l64'
% For the SGI/O2 & HP 700 use 'ieee-be'
% We no longer need to worry about byte order.  The header
% information is in ASCII and the data is written as chars.
   fid = fopen(file_name,'r','ieee-be');
   if ( fid < 0 )
      error( ['Can not open file: ' file_name ] );
   end
   
   hdr = read_property_header( fid );
   if ( hdr.byte_order == 1234 )
      fclose( fid );
      fid = fopen(file_name,'r','ieee-le');
      hdr = read_property_header( fid );
   elseif ( hdr.byte_order == 4321 )
      fclose( fid );
      fid = fopen(file_name,'r','ieee-be');
      hdr = read_property_header( fid );
   end

   if ( ~isempty( col_no ) )
      start_of_data = ftell( fid );
      if ( col_no > hdr.n_y_cols )
         error( [ 'Core position (' ...
                  num2str((col_no*hdr.dy+hdr.ref_y)/1000) ...
                  ' km) out of range.'] );
      end
      for i=1:length(col_no)
         fseek( fid , start_of_data , 'bof' );
         data(:,i) = read_compressed_grid( fid , 'lim' , [col_no(i) 1]*hdr.n_rows )';
      end
   else
      data = read_compressed_grid( fid );
      data = reshape( data , hdr.n_rows , hdr.n_y_cols , hdr.n_x_cols );
   end
   
%   if ( ~isempty( col_no ) )
%      data = data(:,col_no);
%   end
   
   if ( skip ~= 0 )
      dataShort = data(1:skip+1:end,:);
      unit_height = unit_height*(skip+1);
      data = dataShort;
   end

   if ( ~isempty(func) )
      disp( 'filtering the data.' );
      good_i = data>.9*hdr.water_val & data<.9*hdr.rock_val;
      data(good_i) = func( data(good_i) );
   end

   if ( isempty( data_lim ) )
      min_val = min(min(data(data>.9*hdr.water_val)));
      max_val = max(max(data(data<.9*hdr.rock_val)));
   else
      min_val = data_lim(1);
      max_val = data_lim(2);
   end

   disp( ['Maximum value: ' num2str(max_val) ]);
   disp( ['Minimum value: ' num2str(min_val) ]);

   if ( abs( max_val - min_val )/max_val < 1e-5 )
      max_val = max_val+1;
      min_val = min_val-1;
   end
   hdr.min_val = min_val;
   hdr.max_val = max_val;

   if ( ~(isscalar(mask) & ~mask) )
      is_rock         = data>.9*hdr.rock_val;
      data( ~mask )   = hdr.water_val;
      data( is_rock ) = hdr.rock_val;
   end
   
   if ( ~dim )
      disp( 'rescaling the data between 0 and 255.' );
      is_water = data<.9*hdr.water_val;
      is_rock  = data>.9*hdr.rock_val;
      data_uint8 = zeros( size(data,1) , size(data,2) , 'uint8' );
      data_uint8 = (data-min_val)/(max_val-min_val)*252 + 2;
      data_uint8( is_water ) = 0;
      data_uint8( is_rock )  = 255;
      data_uint8( isnan(data)|isinf(data) ) = 1;
      data = data_uint8;
   else
      if ( ~isempty( new_water_val ) )
         data( data<.9*hdr.water_val ) = new_water_val;
      end
      if ( ~isempty( new_rock_val ) )
         data( data>.9*hdr.rock_val ) = new_rock_val;
      end
   end

   fclose(fid);


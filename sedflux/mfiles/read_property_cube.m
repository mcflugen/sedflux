function [data, hdr] = read_property_cube( file_name , varargin )
% READ_PROPERTY   Read a sedflux property file.
%
% DATA = READ_PROPERTY( filename ) Reads the data from the 
%  sedflux property file 'FILE NAME'
%
% SEE ALSO
%

   skip = 0;
   dim = 0;
   col_no = [];
   ascii = 0;
   i = 0;
   while i < length(varargin)
      i = i + 1;
      arg = varargin{i};
      if ( i < nargin-1 )
         next_arg = varargin{i+1};
      else
         next_arg = [];
      end
      if ischar(arg)
         if ( strmatch(upper(arg),'SKIP') )
            skip = next_arg;
            i = i + 1;
         elseif ( strmatch(upper(arg),'FORMAT') )
            format = next_arg;
            i = i + 1;
         elseif ( strmatch(upper(arg),'DIM') )
            dim = 1;
         elseif ( strmatch(upper(arg),'COLS') )
            col_no = next_arg;
            i = i + 1;
         elseif ( strmatch(upper(arg),'ASCII') )
            ascii = 1;
         else
            error(['Invalid parameter']);
         end
      else
         error( ['Input paramter must be a string.'] );
      end
   end

% For the DEC/ALPHA use 'ieee-le.l64'
% For the SGI/O2 & HP 700 use 'ieee-be'
% We no longer need to worry about byte order.  The header
% information is in ASCII and the data is written as chars.
   fid=fopen(file_name,'r');
   
   if ( fid < 0 )
      error_str=sprintf('Can not open file %s',file_name);
      error(error_str);
      data = -1;
      for i=1:nargout
         varargout{i}=-1;
      end
   end
   
   hdr=read_property_header_cube(fid);
   
   if ( ~isempty(col_no) )
      data = nans( [hdr.n_rows length(col_no)]);
      data = get_property_column_cube( fid , col_no );
   else
      data = nans([hdr.n_rows hdr.n_x_cols hdr.n_y_cols]);
      if ( ascii==0 )
         data = fread(fid,[hdr.n_rows hdr.n_x_cols*hdr.n_y_cols],'uchar');
      else
         format = [ repmat( '%d ' , 1 , hdr.n_rows ) '\n' ];
         data = fscanf( fid , format , [hdr.n_rows hdr.n_x_cols*hdr.n_y_cols] );
      end
      data = reshape( data , [hdr.n_rows hdr.n_x_cols hdr.n_y_cols] );
   end
   
   if ( dim == 1 )
      water = data==0;
      rock = data==255;
      data = double(data)*(hdr.max_val-hdr.min_val)/(253) + hdr.min_val;
      data(water) = nan;
      data(rock) = nan;
   else
      data = uint8(data);
   end
   
   fclose(fid);

function col = get_property_column_cube( fid , col_no )

rewind(fid);
hdr = read_property_header_cube(fid);
for i=1:length(col_no)
   seek_property_column_cube( fid , col_no(i) );
   col(:,i) = fread( fid , hdr.n_rows , 'char' );
end

function seek_property_column_cube( fid , col_no )

rewind(fid);
hdr = read_property_header_cube(fid);
fseek( fid , hdr.n_rows*(col_no-1), 'cof' );




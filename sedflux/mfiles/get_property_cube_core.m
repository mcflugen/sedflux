function [core,depth] = get_property_cube_core( filename , position , varargin )
% GET_PROPERTY_CORE   Get a core from a sedflux property file.
%
% GET_PROPERTY_CORE( filename , position )
%
% SEE ALSO 

burial_depth = 'on';

if ( nargin>2 )
   for i=1:2:length(varargin)
      arg1 = varargin{i};
      if ( i+1<=length(varargin) )
         arg2 = varargin{i+1};
      end
      if ( ischar( arg1 ) )
         arg1 = upper(arg1);
         arg2 = upper(arg2);
         if ( strcmp(arg1, 'BURIAL' ) )
            if ( strcmp( arg2 , 'ON' ) )
               burial_depth = 'on';
            elseif ( strcmp( arg2 , 'OFF' ) )
               burial_depth = 'off';
            else
               error( 'Value must be either ''on'' or ''off''. ' );
            end
         else
            error( [ 'Unknown key: ' arg1 ] );
         end
         i = i+1;
      else
         error( 'Key must be a string.' );
      end
   end
end

fid = fopen( filename , 'r' );

hdr = read_property_header_cube( fid );

%%%
%%% Convert position where core will be taken, from km to bin number.
%%%

i = round( position(1,:)*1000/hdr.cell_x_width );
j = round( position(2,:)*1000/hdr.cell_y_width );
index = sub2ind( [hdr.n_y_cols hdr.n_x_cols] , j , i );

data = read_property_cube( filename , ...
                          'col' , index , ...
                          'dim' );

for i=1:size(data,2)
   depth{i} = -([(hdr.n_rows-1):-1:0]*hdr.cell_height + hdr.ref(3));
   core{i}  = data(:,i);

   depth{i}( isnan(core{i}) ) = [];
   core{i}(isnan(core{i})) = [];

   z_max = size(core{i},1);
   if ( strcmp(burial_depth,'off') )
      depth{i} = depth{i}-min(depth{i});
   end
%      depth{i}=[0:(z_max-1)]*hdr.cell_height;
end


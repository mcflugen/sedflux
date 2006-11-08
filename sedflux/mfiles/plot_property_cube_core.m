function h=plot_property_cube_core( filename , position , varargin )
% PLOT_PROPERTY_CORE   plot cores from a sedflux property file.
%
% PLOT_PROPERTY_CORE( FILENAME , POS ) - plot cores from the
% sedflux property file, FILENAME at the positions given by
% the vector, POS.
%
% A third parameter, BURIAL_DEPTH, can also be specified.  If
% BURIAL_DEPTH is 1, grain size is plotted against burial depth,
% if 0, grain size is plotted against water depth.
%
% SEE ALSO get_property_core, read_property.
%

overlay      = 0;
burial_depth = 'on';
sand_val     = [];

if ( nargin>2 )
   for i=1:2:length(varargin)
      arg1 = varargin{i};
      if ( i+1<=length(varargin) )
         arg2 = varargin{i+1};
      end
      if ( ischar( arg1 ) )
         arg1 = upper(arg1);
         arg2 = upper(arg2);
         if ( strcmp(arg1, 'OVERLAY' ) )
            if ( strcmp( arg2 , 'ON' ) )
               overlay = 1;
            elseif ( strcmp( arg2 , 'OFF' ) )
               overlay = 0;
            else
               error( 'Value must be either ''on'' or ''off''. ' );
            end
         elseif ( strcmp(arg1, 'BURIAL' ) )
            if ( strcmp( arg2 , 'ON' ) )
               burial_depth = 'on';
            elseif ( strcmp( arg2 , 'OFF' ) )
               burial_depth = 'off';
            else
               error( 'Value must be either ''on'' or ''off''. ' );
            end
         elseif ( strcmpi( arg1 , 'SAND' ) )
            if ( isnumeric( arg2 ) & prod(size(arg2))==1 )
               sand_val = arg2;
            else
               error( 'Sand value must be a scalar value.' );
            end
         else
            error( ['Unknown key: ' arg1] );
         end
      else
         error( 'Key must be a string.' );
      end
   end
end

if ( overlay )
   burial_depth = 'on';
end

[ core depth ] = get_property_cube_core( filename , position , ...
                                         'BURIAL' , burial_depth );

for i=1:length(core)
   c_order = get(gca,'colororder');
   if ( overlay )
      hold on
%      h(i) = plot( core{i}+position(i)-mean(core{i}) , depth{i} , ...
%                  'color' , c_order(i,:) );
      h(i) = plot( core{i}+position(1,i)-mean(core{i}) , ...
                    depth{i} , ...
                    'color' , c_order(i,:) );
      set(gca,'YDir','reverse');
   else
      h(i) = plot( core{i} , depth{i} , ...
                  'color' , c_order(i,:) );
      set(gca,'YDir','reverse');
   end
   hold on
end

if ( ~overlay )
   xlabel('Grain Size (\phi)');
end
if ( strcmp(burial_depth,'on') )
   ylabel('Burial Depth (m)');
else
   ylabel('Water Depth (m)');
end

% We need position to be a column vector.
position = shiftdim(position);

labels = num2str(position');
labels = [ char(ones(size(labels,1),1))*'(' ...
           labels ...
           char(ones(size(labels,1),1)*') km') ];
legend(labels)

if ( overlay )
   for i=1:length(core)
      plot( [ position(1,i) position(1,i) ] , ...
            [ min(depth{i}) max(depth{i}) ] , ...
            'k' )
   end
end

hold off


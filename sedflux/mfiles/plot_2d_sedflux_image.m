%% Plot a sedflux property image
%
% Plot a sedflux profile as a 2D image.
%
% Optional parameter/value pairs:
%   - xlabel : Label for the x-axis. [string]
%   - ylabel : Label for the y-axis. [string]
%   - zlabel : Label for the z-axis. [string]
%   - xslice : Plot image along constant x-slice. [Double]
%   - yslice : Plot image along constant y-slice. [Double]
%   - zslice : Plot image along constant z-slice. [Double]
%   - property : The name of the property to image. [String]
%   - sealevel : Elevation of sea level. [Double]
%   - colorbar : Indicate that a colorbar should be included. [boolean]
%   - time : The time of the image. [double]
%   - clim : Scale colors between these limits. [vector double]
%
% \param x         X-locations of the pixel centers of C
% \param y         Y-locations of the pixel centers of C
% \param z         Z-locations of the pixel centers of C
% \param c         Matrix to image (as in the Matlab image command)
% \param varargin  Parameter/value pairs
% 
% \returns A handle to the new image
%

function h_img = plot_2d_sedflux_image( x , y , z , c , varargin )

axis_font_size  = 12;
title_font_size = 14;
label_font_size = 12;

colorbar_is_on = true;

valid_args = { 'xlabel'     , 'char'   , [] ; ...
               'ylabel'     , 'char'   , [] ; ...
               'zlabel'     , 'char'   , [] ; ...
               'xslice'     , 'double' , [] ; ...
               'yslice'     , 'double' , [] ; ...
               'zslice'     , 'double' , [] ; ...
               'property'   , 'char'   , [] ; ...
               'sealevel'   , 'double' , [] ; ...
               'colorbar'   , 'logical' , true ; ...
               'time'       , 'double' , [] ; ...
               'clim'       , 'double' , [] };

values = parse_varargin( valid_args , varargin );

x_label     = values{strmatch( 'xlabel'   , {valid_args{:,1}} , 'exact' )};
y_label     = values{strmatch( 'ylabel'   , {valid_args{:,1}} , 'exact' )};
z_label     = values{strmatch( 'zlabel'   , {valid_args{:,1}} , 'exact' )};
x_slice     = values{strmatch( 'xslice'   , {valid_args{:,1}} , 'exact' )};
y_slice     = values{strmatch( 'yslice'   , {valid_args{:,1}} , 'exact' )};
z_slice     = values{strmatch( 'zslice'   , {valid_args{:,1}} , 'exact' )};
property_id = values{strmatch( 'property' , {valid_args{:,1}} , 'exact' )};
sea_level   = values{strmatch( 'sealevel' , {valid_args{:,1}} , 'exact' )};
colorbar_is_on = values{strmatch( 'colorbar' , {valid_args{:,1}} , 'exact' )};
time        = values{strmatch( 'time'     , {valid_args{:,1}} , 'exact' )};
data_lim    = values{strmatch( 'clim'     , {valid_args{:,1}} , 'exact' )};

if ( isempty( z ) )
   h_img = image( x , y , c );
else
   h_img = slice( x , y , z , double(c) , ...
                  x_slice , y_slice , z_slice );
   set(h_img,'edgecolor','none')
end

h_fig = gcf;
h_ax  = gca;

set( h_img , 'tag' , 'sedflux-2d-image' );

if ( ~isempty(sea_level) & isempty(z) )
   hold on
   plot( get(h_ax,'xlim') , zeros(1,2) , 'k' );
   hold off
end

[property_name , units] = get_property_full_name( property_id );

title( ['Cross-section of ' property_name] , 'fontsize' , title_font_size );
set( h_ax , 'FontSize' , axis_font_size );
set( h_ax , 'ydir' , 'reverse' );

if ( ~isempty(x_label) ), xlabel( x_label , 'fontsize' , label_font_size ); end
if ( ~isempty(y_label) ), ylabel( y_label , 'fontsize' , label_font_size ); end
if ( ~isempty(z_label) ), zlabel( z_label , 'fontsize' , label_font_size ); end

map = get_property_colormap( property_id );
set( h_fig , 'colormap' , map );

%if ( ~isempty( colorbar_is_on ) )
if ( colorbar_is_on )

      % If this already is a sedflux-2d-image and there is a colorbar, then
      % there is no need to change the height of the image as it should
      % already have been done.
      if ( isempty( findobj( gcf , 'Tag' , 'Colorbar' )  ) )
         pos = get(h_ax,'position');
         set(h_ax,'position',[ pos(1) (pos(2)+pos(4)*.2) pos(3) .8*pos(4) ] );
      end

      h_bar = subplot(20,1,20);
      reset(h_bar)
      h_bar = colorbar(h_bar);
      axes(h_bar);
      child = findobj(h_bar,'type','image');
      if ( data_lim(1) == data_lim(2) )
      	data_lim(1) = data_lim(1)-1;
      	data_lim(2) = data_lim(2)+1;
      end
      set(child,'xdata',data_lim)

      eps = (data_lim(2) - data_lim(1) ) / size(colormap,1);
      set( h_bar,'XLim',[data_lim(1)+2*eps data_lim(2)-eps]);

      set( h_bar , 'FontSize' , axis_font_size );
      xlabel( [property_name ' (' units ')'] , 'fontsize' , label_font_size );
end

axes( h_ax );

if ( ~isempty( time ) )
   text( .95 , .9 , [ 'Time = ' num2str(time) ' years' ] , ...
         'units' , 'normalized' , 'fontsize' , label_font_size , ...
         'horizontalalignment' , 'right' );
end




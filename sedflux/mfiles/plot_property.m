%%
% \brief Plot a SEDFLUX property file.
%
% Plot a sedflux output property file.  If the property file is
% a data cube that was generated with sedflux3D, property
% profiles can be plotted in a fence diagram or as single profiles.
%
% A series of parameter value pairs can be passed that control
% the way the data are presented.  Possible parameters are:
%  - SEDFLUX3D
%   -# xslice   : Plot the profiles with constant x-values.  X-values
%                 are given in units of kilometers. [scalar or vector]
%   -# yslice   : Plot the profiles with constant y-values.  Y-values
%                 are given in units of kilometers. [scalar or vector]
%   -# zslice   : Plot the profiles with constant z-values.  Z-values
%                 are given in units of meters. [scalar or vector]
%   -# nxslices : Plot a given number of x-profiles. [scalar]
%   -# nyslices : Plot a given number of y-profiles. [scalar]
%   -# nzslices : Plot a given number of z-profiles. [scalar]
%  - Both SEDFLUX3D and SEDFLUX2D
%   -# time     : Give the time associated with this profile.  A label
%                 is placed on the image that gives this time.  This is
%                 typically used when making a movie from a series of
%                 profiles. [scalar]
%   -# clim     : Define the limits of the property. [vector]
%   -# mask     : Logical array that indicates which values should be
%                 imaged and which should be masked.  The size of the
%                 mask should be the same as that of the image.  [Vector]
%   -# func     : Function called to process the data before plotting.
%                 [function_handle]
%   -# colorbar : Logical value that indicates whether to include a
%                 colorbar. [scalar]
%   -# nofloor  : Logical value that indicated whether to remove the
%                 bedrock from the image. [scalar]
%
% @param filename    Name of the property file
% @param varargin    A series of parameter/value pairs
%
function plot_property( filename , varargin )

% PLOT_PROPERTY   Plot a sedflux property file.
%
% PLOT_PROPERTY( FILENAME ) - plots the sedflux property file
% FILENAME as an image in the current axis.
%
% FILENAME can be followed by parameter/value pairs to specify
% additional properties of the image.
%
   valid_args = { 'xslice'     , 'double' , [] ; ...
                  'yslice'     , 'double' , [] ; ...
                  'zslice'     , 'double' , [] ; ...
                  'nxslices'   , 'double' , 5 ; ...
                  'nyslices'   , 'double' , 5 ; ...
                  'nzslices'   , 'double' , 0 ; ...
                  'time'       , 'double' , [] ; ...
                  'clim'       , 'double' , [] ; ...
                  'mask'       , 'logical' , false ; ...
                  'func'       , 'function_handle' , [] ; ...
                  'colorbar'   , 'logical' , true ; ...
                  'nofloor'    , 'logical' , false };

   values = parse_varargin( valid_args , varargin );

   x_slice        = values{strmatch( 'xslice'   , {valid_args{:,1}} , 'exact' )};
   y_slice        = values{strmatch( 'yslice'   , {valid_args{:,1}} , 'exact' )};
   z_slice        = values{strmatch( 'zslice'   , {valid_args{:,1}} , 'exact' )};
   n_x_slices     = values{strmatch( 'nxslices' , {valid_args{:,1}} , 'exact' )};
   n_y_slices     = values{strmatch( 'nyslices' , {valid_args{:,1}} , 'exact' )};
   n_z_slices     = values{strmatch( 'nzslices' , {valid_args{:,1}} , 'exact' )};
   time           = values{strmatch( 'time'     , {valid_args{:,1}} , 'exact' )};
   clim           = values{strmatch( 'clim'     , {valid_args{:,1}} , 'exact' )};
   mask           = values{strmatch( 'mask'     , {valid_args{:,1}} , 'exact' )};
   func           = values{strmatch( 'func'     , {valid_args{:,1}} , 'exact' )};
   colorbar_is_on = values{strmatch( 'colorbar' , {valid_args{:,1}} , 'exact' )};
   no_floor       = values{strmatch( 'nofloor'  , {valid_args{:,1}} , 'exact' )};

   if ( isempty( func ) )
      [data , header] = read_property( filename , ...
                                       'dim'  , false , ...
                                       'clim' , clim  , ...
                                       'mask' , mask );
   else
      [data , header] = read_property( filename , ...
                                       'dim'  , false , ...
                                       'clim' , clim  , ...
                                       'mask' , mask  , ...
                                       'func' , func );
   end

   if ( isempty(clim) )
      clim = [ header.min_val header.max_val ];
   end
   
   if ( length( size(data) ) == 2 )
      is_sedflux_2d = true;
      [rows y_cols] = size(data);
   else
      is_sedflux_2d = false;
      [rows x_cols y_cols] = size(data);
   end

   x_data = ([0:(header.n_x_cols-1)]*header.dx + header.ref_x)/1000.;
   y_data = ([0:(header.n_y_cols-1)]*header.dy + header.ref_y)/1000.;
   z_data = [(header.n_rows-1):-1:0]*header.cell_height + header.ref_z;
   z_data = -z_data + header.sea_level;

   data = data+1;
   data( data==2 ) = 3;

   sea_level = find(z_data>=0);
   if ( ~isempty(sea_level) )
      sea_level = sea_level(1);
      is_air = zeros( size(data) , 'uint8' );
      is_air( 1:sea_level,:,: ) = 1;
      data( logical(is_air) & data==1 ) = 2;
   end

   if ( no_floor )
      disp( 'Removing the basement' );
      for i=1:(size(data,2)*size(data,3))
         col=data(:,i);
         col(col==2) = 1;
         ind = find(col~=256);
         data(:,i) = 1;
         data((end-length(ind)+1):end,i) = col(ind);
      end
   end

   if (   sum( size([x_slice y_slice z_slice]) ) > 2 ...
        | isempty( [x_slice y_slice z_slice] ) ...
        & ~is_sedflux_2d )
      data = double(data);
      data(data==1 | data==2) = nan;
%      data(data==256) = nan;
   end

   if ( is_sedflux_2d )
      plot_2d_sedflux_image( y_data , z_data , [] , data , ...
                             'property' , header.property , ...
                             'xlabel'   , 'Y-Distance (km)' , ...
                             'ylabel'   , 'Depth (m)' , ...
                             'sealevel' , header.sea_level , ...
                             'time'     , time , ...
                             'colorbar' , colorbar_is_on , ...
                             'clim'     , clim );
   else

      data=permute(data,[3 2 1]);
      [mesh_y,mesh_x,mesh_z]=meshgrid( y_data , x_data , z_data );

      if ( length(x_slice) == 1 & isempty( [y_slice z_slice] ) )
         zi = interp3( mesh_y , mesh_x , mesh_z , double(data) , ...
                       y_data , x_slice , z_data );
   
         plot_2d_sedflux_image( y_data , z_data , [] , squeeze(zi)' , ...
                                'property' , header.property , ...
                                'xlabel'   , 'Y-Distance (km)' , ...
                                'ylabel'   , 'Depth (m)' , ...
                                'sealevel' , header.sea_level , ...
                                'time'     , time , ...
                                'colorbar' , colorbar_is_on , ...
                                'clim'     , clim );
      elseif ( length(y_slice) == 1 & isempty( [x_slice z_slice] ) )
         zi = interp3( mesh_y , mesh_x , mesh_z , double(data) , ...
                       y_slice , x_data , z_data );
   
         plot_2d_sedflux_image( x_data , z_data , [] , squeeze(zi)' , ...
                                'property' , header.property , ...
                                'xlabel'   , 'X-Distance (km)' , ...
                                'ylabel'   , 'Depth (m)' , ...
                                'sealevel' , header.sea_level , ...
                                'time'     , time , ...
                                'colorbar' , colorbar_is_on , ...
                                'clim'     , clim );
      elseif ( length(z_slice) == 1 & isempty( [x_slice y_slice] ) )
         zi = interp3( mesh_y , mesh_x , mesh_z , double(data) , ...
                         y_data , x_data , z_slice );
   
         plot_2d_sedflux_image( x_data , y_data , [] , squeeze(zi) , ...
                                'property' , header.property , ...
                                'xlabel'   , 'Y-Distance (km)' , ...
                                'ylabel'   , 'X-Distance (km)' , ...
                                'time'     , time , ...
                                'colorbar' , colorbar_is_on , ...
                                'clim'     , clim );
         set( gca , 'DataAspectRatio' , [1 1 1] );
      else
   
         if ( isempty( x_slice ) & n_x_slices > 0 )
            x_slice = linspace( min(x_data) , max(x_data) , n_x_slices );
         end
         if ( isempty( y_slice ) & n_y_slices > 0 )
            y_slice = linspace( min(y_data) , max(y_data) , n_y_slices );
         end
         if ( isempty( z_slice ) & n_z_slices > 0 )
            z_slice = linspace( min(z_data) , max(z_data) , n_z_slices );
         end
   
         plot_2d_sedflux_image( mesh_y , mesh_x , mesh_z , double(data) , ...
                                'xslice' , y_slice , ...
                                'yslice' , x_slice , ...
                                'zslice' , z_slice , ...
                                'property' , header.property , ...
                                'xlabel' , 'X-Distance (km)' , ...
                                'ylabel' , 'Y-Distance (km)' , ...
                                'zlabel' , 'Depth (m)' , ...
                                'time'     , time , ...
                                'colorbar' , colorbar_is_on , ...
                                'clim'     , clim );
         set( gca , 'zdir' , 'rev' );
         view( 37.5 , 45 )
%         camlight( -37.5 , 30 )
         camlight( 90 , 0 )

      end
   end

   set(gcf,'NumberTitle','off','Name',[ pwd '/' filename]);


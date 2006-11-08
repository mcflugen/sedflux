function ax = plot_measuring_station_surface( filename , varargin )
% PLOT_MEASURING_STATION   Plot a Wheeler type diagram from sedflux data.
%
% PLOT_MEASURING_STATION( filename )

valid_args = { 'timeslice'  , 'double' , inf ; ...
               'sealevel'   , 'double' , [] ; ...
               'size'       , 'double' , [] ; ...
               'zlim'       , 'double' , [] ; ...
               'wheeler'    , 'logical', false ; ...
               'colorbar'   , 'logical', true ; ...
               'time'       , 'logical', true ; ...
               'view'       , 'double' , [] ; ...
               'diff'       , 'logical' , false ; ...
               'surf'       , 'logical' , true ; ...
               'refslice'   , 'double'  , [] ; ...
               'hook'       , 'function_handle' , @empty_hook ; ...
               'zdir'       , 'char'   , 'rev' };

values = parse_varargin( valid_args , varargin );

time_slice     = values{strmatch( 'timeslice', {valid_args{:,1}} , 'exact' )};
sea_level      = values{strmatch( 'sealevel' , {valid_args{:,1}} , 'exact' )};
grid_size      = values{strmatch( 'size'     , {valid_args{:,1}} , 'exact' )};
z_lim          = values{strmatch( 'zlim'     , {valid_args{:,1}} , 'exact' )};
time           = values{strmatch( 'time'     , {valid_args{:,1}} , 'exact' )};
z_dir          = values{strmatch( 'zdir'     , {valid_args{:,1}} , 'exact' )};
use_this_view  = values{strmatch( 'view'     , {valid_args{:,1}} , 'exact' )};
colorbar_is_on = values{strmatch( 'colorbar' , {valid_args{:,1}} , 'exact' )};
time_diff      = values{strmatch( 'diff'     , {valid_args{:,1}} , 'exact' )};
surface_plot   = values{strmatch( 'surf'     , {valid_args{:,1}} , 'exact' )};
ref_slice      = values{strmatch( 'refslice' , {valid_args{:,1}} , 'exact' )};
wheeler        = values{strmatch( 'wheeler'  , {valid_args{:,1}} , 'exact' )};
hook_func      = values{strmatch( 'hook'     , {valid_args{:,1}} , 'exact' )};

if ( ~isempty( ref_slice ) )
   [z_0,loc,t,hdr] = read_measuring_station_cube( filename    , ...
                                                  'timeslice' , ref_slice );
end

[z,loc,t,hdr] = read_measuring_station_cube( filename    , ...
                                             'timeslice' , time_slice , ...
                                             'diff'      , time_diff );

[full_name, units] = get_property_full_name( hdr.property );

x = reshape(loc(1,:), size(z) )/1000;
y = reshape(loc(2,:), size(z) )/1000;

if ( ~isempty(sea_level) & size(sea_level)~=1 )
   this_sea_level = interp1(sea_level(1,:),sea_level(2,:),t);
   if ( ~isnan( this_sea_level ) )
      if ( strcmp( full_name , 'Water Depth' ) )
         z = z + this_sea_level;
      end
   else
      this_sea_level = 0;
      disp( 'The given sea-level curve does not contain this time slice.' );
      disp( [ 'The provided sea level runs from ' ...
              num2str(min(sea_level(1,:))) ...
              ' years to ' num2str(max(sea_level(1,:))) ' years.' ] );
      disp( ['The current time slice is at ' num2str(t) ' years.' ] );
      disp( 'Setting sea level to 0.' );
   end
elseif ( size(sea_level)==1 )
   this_sea_level = sea_level;
else
   this_sea_level = 0;
end

z = squeeze(z);

if ( ~isempty(ref_slice) )
   z = z - z_0;
end

if ( isempty( z_lim ) )
   z_lim = [min(min(z)) max(max(z))];
end

if ( hdr.n_x_cols > 1 )

   if ( surface_plot )

      h_img = surf( y,x,z );

      if ( ~isempty( z_lim ) )
         caxis( z_lim );
      end

      h_ax  = gca;
      set(h_img,'edgecolor','none');
      set(h_img,'facecolor','interp');

      axis tight;

      if ( ~isempty( strmatch( z_dir , { 'rev' , 'norm' } ) ) )
         set( h_ax , 'zdir' , z_dir );
      else
         error( ['Unknown value for parameter ''zdir'': ' z_dir '.  ' ...
                 'Expecting ''rev'' or ''norm''.'] );
      end
   
      set(h_ax,'ydir','rev')
      set(h_ax,'xdir','rev')
   
      lighting phong
      if ( ~isempty( use_this_view ) )
         view( use_this_view );
      else
         view( [-30 60] );
      end
%      camlight( -30 , 60 )
      camlight headlight

      if ( ~isempty(z_lim) )
         zlim( z_lim );
      end

      zlabel( [full_name ' (' units ')' ]);
   else
      h_img = imagesc( y(1,:),x(:,1),z );
      if ( ~isempty( z_lim ) )
         caxis( z_lim );
      end

      h_ax  = gca;
   end

   set(h_ax,'dataaspectratio',[1 1 2]);
   xlabel( 'Y-Distance (km)' , 'FontSize' , 12 );
   ylabel( 'X-Distance (km)' , 'FontSize' , 12 );

else

   if ( wheeler )
      imagesc( squeeze(y(:,1,end)) , t , z' );
      ylabel( 'Time (years)' )
   else
      plot( squeeze(y) , z );

      if ( time_diff )
         ylabel( ['Time Derivative of ' full_name ' (' units ')' ]);
      else
         ylabel( [full_name ' (' units ')' ]);
      end

      if ( ~isempty(z_lim) )
         ylim( z_lim );
      end

   end

   h_ax = gca;
%   xlabel( 'Y-Distance (km)' );
%   ylabel( [full_name ' (' units ')' ]);

   xlabel( 'Y-Distance (km)' );

end

if ( time )
   title( ['Time: ' num2str(t) ' years'] , 'FontSize' , 12 );
end

if ( wheeler | hdr.n_x_cols > 1 )
   colormap( land_water_map( gca , 'sealevel' , this_sea_level , ...
                                   'zdir'     , z_dir ) );
%   colormap( hot );

   if ( colorbar_is_on )
      h_bar=colorbar( 'horiz' );
      axes(h_bar)
      xlabel( [full_name ' (' units ')' ] , 'FontSize' , 12 );
   end
end

if ( ~isempty(sea_level) )

   if ( hdr.n_x_cols == 1 & strcmp( full_name , 'Topographic Elevation' ) )
      hold on
      plot( get(gca,'xlim') , [this_sea_level this_sea_level] , 'b' );
      hold off
   end

   if ( size(sea_level)~=1 )
      add_sea_level_plot( sea_level , 'time' , t );
   end

end

axes( h_ax );

if ( ~isempty(hook_func) )
   hook_func( h_ax );
end

function empty_hook( ax )


%% Create a MATLAB movie of a measuring station file
%
% Optional paramter/value pairs:
%   - sealevel : Elevation of sea level. [double]
%   - holdframe : Once this frame is plotted, overlay subsequent
%                 plots. [boolean]
%   - zlim : Specify the limits of the axis in the z-direction. [vector double]
%   - zdir : Specify the direction of the z-axis.  Either 'norm' or 'rev'. [string]
%   - view : Specify the camera view. [vector double]
%   - diff : Take time derivatives of measurments. [boolean]
%   - surf : Plot data using surf command rather than image. [boolean]
%   - refslice : Specify a reference time slice from which all data are
%                differenced from. [int]
%   - hook : Function to run after the data are plotted.  The function should accept
%            a handle to an axis as its only input parameter. [function_handle]
%   - skip : Skip this number of measurements between frames. [int]
%
function M=film_measuring_station_surface( filename , varargin )
% FILM_PROPERTY_SLICE   Make a film from a series of sedflux slices.
%
%

valid_args = { 'sealevel'     , 'double'  , [] ; ...
               'holdframe'    , 'double'  , [] ; ...
               'timestep'     , 'double'  , [] ; ...
               'size'         , 'double'  , [] ; ...
               'zlim'         , 'double'  , [] ; ...
               'zdir'         , 'char'    , 'rev' ; ...
               'view'         , 'double'  , [] ; ...
               'diff'         , 'logical' , false ; ...
               'surf'         , 'logical' , true ; ...
               'refslice'     , 'double'  , [] ; ...
               'hook'         , 'function_handle' , @empty_hook ; ...
               'skip'         , 'double'  , 0 };

values = parse_varargin( valid_args , varargin );

sea_level     = values{strmatch( 'sealevel'  , {valid_args{:,1}} , 'exact' )};
hold_frame    = values{strmatch( 'holdframe' , {valid_args{:,1}} , 'exact' )};
time_step     = values{strmatch( 'timestep'  , {valid_args{:,1}} , 'exact' )};
grid_size     = values{strmatch( 'size'      , {valid_args{:,1}} , 'exact' )};
z_lim         = values{strmatch( 'zlim'      , {valid_args{:,1}} , 'exact' )};
z_dir         = values{strmatch( 'zdir'      , {valid_args{:,1}} , 'exact' )};
use_this_view = values{strmatch( 'view'      , {valid_args{:,1}} , 'exact' )};
time_diff     = values{strmatch( 'diff'      , {valid_args{:,1}} , 'exact' )};
surface_plot  = values{strmatch( 'surf'      , {valid_args{:,1}} , 'exact' )};
ref_slice     = values{strmatch( 'refslice'  , {valid_args{:,1}} , 'exact' )};
hook_func     = values{strmatch( 'hook'      , {valid_args{:,1}} , 'exact' )};
skip          = values{strmatch( 'skip'      , {valid_args{:,1}} , 'exact' )};

n_records = get_measuring_station_n_records( filename )
if ( isempty(z_lim) )
   data = read_measuring_station_cube( filename , ...
                                       'skip'   , skip , ...
                                       'diff'   , time_diff );
%   z_lim = [min(-data(:)) max(-data(:))]
   if ( ~isempty( ref_slice ) )
      data_0 = read_measuring_station_cube( filename , ...
                                            'timeslice' , ref_slice );
      data = data - repmat( data_0 , [1 1 size(data,3)] );
   end
   z_lim = [min(data(:)) max(data(:))]
   clear data;
end

plot_measuring_station_surface( filename    , ...
                                'timeslice' , 1 , ...
                                'sealevel'  , sea_level , ...
                                'view'      , use_this_view , ...
                                'diff'      , time_diff , ...
                                'surf'      , surface_plot , ...
                                'refslice'  , ref_slice , ...
                                'zlim'      , z_lim , ...
                                'hook'      , hook_func , ...
                                'zdir'      , z_dir );
hold_frame = sort( hold_frame );
if ( ~isempty(hold_frame) & hold_frame(1) <= 1 )
   hold_frame(1) = [];
   h = get(gca,'child');
   frame{1}.x = get( h(end) , 'xdata' );
   frame{1}.y = get( h(end) , 'ydata' );
else
   frame = {};
end

%if ( ~isempty(sea_level) )
%   add_sea_level_plot( sea_level , 'time' , 0 )
%end
set( gcf , 'PaperPositionMode' , 'auto' );

M(1) = getframe( gcf );
i = 1;
%hold on
for n=(skip+1):(skip+1):n_records
   cla

   hold on

   for j=1:length(frame)
      plot( frame{j}.x , frame{j}.y , 'm' )
   end

   plot_measuring_station_surface( filename    , ...
                                   'timeslice' , n , ...
                                   'sealevel'  , sea_level , ...
                                   'time'      , true , ...
                                   'view'      , use_this_view , ...
                                   'diff'      , time_diff , ...
                                   'surf'      , surface_plot , ...
                                   'refslice'  , ref_slice , ...
                                   'zlim'      , z_lim , ...
                                   'hook'      , hook_func , ...
                                   'zdir'      , z_dir );

   hold off

   if ( ~isempty(hold_frame) & n >= hold_frame(1) )
      hold_frame(1) = [];

      h         = get(gca,'child');
      frame_len = length(frame);
      h         = h( length(h)-frame_len );
      frame{frame_len+1}.x = get( h , 'xdata' );
      frame{frame_len+1}.y = get( h , 'ydata' );
   end

%   if ( ~isempty(sea_level) )
%      add_sea_level_plot( sea_level , 'time' , n*time_step );
%   end
   M(i)=getframe( gcf );
   i = i+1;
end

function n_records = get_measuring_station_n_records( filename )

fid   = fopen( filename , 'r' );
hdr   = read_tripod_header( fid );

n     = fread( fid , 1 , 'int32' );

data_start = ftell( fid );
fseek( fid , 0 , 'eof' );
data_end   = ftell( fid );

fclose(fid);

n_records = (data_end-data_start)/(n*8);

function empty_hook( ax )


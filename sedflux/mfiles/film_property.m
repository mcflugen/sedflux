function M=film_property( filefilter , varargin )
% FILM_PROPERTY   Make a film from a series of sedflux property files.
%
% Valid args:
%    xslice, yslice, zslice
%    nxslices, nyslices, nzslices
%    time
%    timestep
%    sealevel
%    clim
%    xlim, ylim, zlim
%    func
%    colorbar
%    nofloor

filename = [];

if ( iscell( filefilter ) )
   for i=1:length(filefilter)
      file = dir( filefilter );

      for n=1:length(file)
         filename{end+1} = file(n).name;
      end
      filename = sort(filename);
   end
   filename = sort(filename);
else
   file = dir(filefilter);

   for n=1:length(file)
      filename{n} = file(n).name;
   end
   filename = sort(filename);
end

valid_args = { 'xslice'     , 'double' , [] ; ...
               'yslice'     , 'double' , [] ; ...
               'zslice'     , 'double' , [] ; ...
               'nxslices'   , 'double' , 5 ; ...
               'nyslices'   , 'double' , 5 ; ...
               'nzslices'   , 'double' , 0 ; ...
               'time'       , 'double' , [] ; ...
               'timestep'   , 'double' , [] ; ...
               'sealevel'   , 'double' , [] ; ...
               'clim'       , 'double' , [] ; ...
               'xlim'       , 'double' , [] ; ...
               'ylim'       , 'double' , [] ; ...
               'zlim'       , 'double' , [] ; ...
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
time_step      = values{strmatch( 'timestep' , {valid_args{:,1}} , 'exact' )};
sea_level      = values{strmatch( 'sealevel' , {valid_args{:,1}} , 'exact' )};
clim           = values{strmatch( 'clim'     , {valid_args{:,1}} , 'exact' )};
x_lim          = values{strmatch( 'xlim'     , {valid_args{:,1}} , 'exact' )};
y_lim          = values{strmatch( 'ylim'     , {valid_args{:,1}} , 'exact' )};
z_lim          = values{strmatch( 'zlim'     , {valid_args{:,1}} , 'exact' )};
func           = values{strmatch( 'func'     , {valid_args{:,1}} , 'exact' )};
colorbar_is_on = values{strmatch( 'colorbar' , {valid_args{:,1}} , 'exact' )};
no_floor       = values{strmatch( 'nofloor'  , {valid_args{:,1}} , 'exact' )};

if ( isempty( z_lim ) )
   top_row    = -inf;
   bottom_row = inf;
   for n=1:length(filename)
      fid = fopen(filename{n});
      header = read_property_header(fid);
      fclose(fid);
%   if ( header.ref(2) < bottom_row )
%      header.ref_z = header.ref_z + header.sea_level;
      header_sea_level(n) = header.sea_level;
      if ( header.ref_z < bottom_row )
         bottom_row = header.ref_z;
      end
      if ( header.ref_z*header.cell_height > top_row )
         top_row = header.ref_z + header.n_rows*header.cell_height;
      end
   end
else

   for n=1:length(filename)
      fid = fopen(filename{n});
      header = read_property_header(fid);
      fclose(fid);

      header_sea_level(n) = header.sea_level;
   end

   bottom_row = z_lim(1);
   top_row    = z_lim(2);
end

top_row
bottom_row

plot_property(filename{1},'colorbar', false , 'time' , 0 , 'clim' , clim );
if ( ~isempty(sea_level) )
   add_sea_level_plot( sea_level , 'time' , 0 )
end
set( gcf , 'PaperPositionMode' , 'auto' );

%ylim([bottom_row top_row])
ylim( [-top_row -bottom_row]+header_sea_level(1) )

if ( ~isempty( x_lim ) )
   xlim( x_lim );
end

M(1) = getframe( gcf );
hold on
for n=2:length(filename)
   cla
   plot_property(filename{n},'colorbar', false , 'time' , n*time_step , 'clim' , clim );
%   ylim([bottom_row top_row])
%   ylim([-top_row -bottom_row])
ylim( [-top_row -bottom_row]+header_sea_level(n) )
   if ( ~isempty( x_lim ) )
      xlim( x_lim );
   end

   if ( ~isempty(sea_level) )
      add_sea_level_plot( sea_level , 'time' , n*time_step );
   end
   M(n)=getframe( gcf );
end


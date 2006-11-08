function M=film_property_slice( filefilter , varargin )
% FILM_PROPERTY_SLICE   Make a film from a series of sedflux slices.
%
%

filename = [];

if ( iscell( filefilter ) )
   for i=1:length(filefilter)
      file = ls(filefilter{i});
      [next_file file]=strtok( file );
      while ( ~isempty( next_file ) )
         filename{end+1} = next_file;
         [next_file file]=strtok( file );
      end
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
               'sealevel'   , 'double' , [] ; ...
               'timestep'   , 'double' , [] };

values = parse_varargin( valid_args , varargin );

x_slice    = values{strmatch( 'xslice'   , {valid_args{:,1}} , 'exact' )};
y_slice    = values{strmatch( 'yslice'   , {valid_args{:,1}} , 'exact' )};
z_slice    = values{strmatch( 'zslice'   , {valid_args{:,1}} , 'exact' )};
sea_level  = values{strmatch( 'sealevel' , {valid_args{:,1}} , 'exact' )};
time_step  = values{strmatch( 'timestep' , {valid_args{:,1}} , 'exact' )};

top_row = -inf;
bottom_row = inf;
for n=1:length(filename)
   fid = fopen(filename{n});
   header = read_property_header_cube(fid);
   fclose(fid);
   if ( header.ref(3) < bottom_row )
      bottom_row = header.ref(3);
   end
   if ( header.ref(3)*header.cell_height > top_row )
      top_row = header.ref(3) + header.n_rows*header.cell_height;
   end
end

plot_property_slice( filename{1},'Colorbar','off' , 'time' , 0 , ...
                     'xslice' , x_slice , ...
                     'yslice' , y_slice , ...
                     'zslice' , z_slice );
if ( ~isempty(sea_level) )
   add_sea_level_plot( sea_level , 'time' , 0 )
end
set( gcf , 'PaperPositionMode' , 'auto' );

%ylim([bottom_row top_row])
ylim([-top_row -bottom_row])

M(1) = getframe( gcf );
hold on
for n=2:length(filename)
   cla
   plot_property_slice(filename{n},'Colorbar','off' , 'time' , n*time_step , ...
                     'xslice' , x_slice , ...
                     'yslice' , y_slice , ...
                     'zslice' , z_slice );
   if ( ~isempty(sea_level) )
      add_sea_level_plot( sea_level , 'time' , n*time_step );
   end
   M(n)=getframe( gcf );
end


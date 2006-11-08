function M=film_property( filefilter , varargin )
% FILM_PROPERTY   Make a film from a series of sedflux property files.
%
%

filename = [];

if ( iscell( filefilter ) )
   for i=1:length(filefilter)
      file = dir(filefilter{i});

%      [next_file file]=strtok( file );
%      while ( ~isempty( next_file ) )
%         filename{end+1} = next_file;
%         [next_file file]=strtok( file );
%      end

      for n=1:length(file)
         filename{end+1} = file(n).name;
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

time_step = [];
sea_level = [];

if nargin>1
   for i=1:length(varargin)
      arg = varargin{i};
      if ( i<length(varargin) )
         next_arg = varargin{i+1};
      end
      if ( ischar(arg) )
         arg = upper(arg);
         if ( strcmp( arg , 'TIMESTEP' ) )
            if ( max(size(next_arg))==1 )
               time_step = next_arg;
            else
               error( 'Time step must be a scalar.' );
            end
            i = i+1;
         elseif ( strcmp( arg , 'SEALEVEL' ) )
            if ( length( size(next_arg) )==2 & min( size(next_arg) )==2 )
               if ( size(next_arg,1)~=2 )
                  next_arg=shiftdim( next_arg , 1 );
               end
               sea_level = next_arg;
            else
               error( 'Sea level must be a 2D array 2 rows or columns.' );
            end
            i = i+1;
         else
            error( 'Invalid key' );
         end
      end
   end
end

top_row = -inf;
bottom_row = inf;
for n=1:length(filename)
   fid = fopen(filename{n});
   header = read_property_header_old(fid);
   fclose(fid);
   if ( header.ref(2) < bottom_row )
      bottom_row = header.ref(2);
   end
   if ( header.ref(2)*header.cell_height > top_row )
      top_row = header.ref(2) + header.n_rows*header.cell_height;
   end
end

plot_property_old(filename{1},'Colorbar','off' , 'time' , 0 );
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
   plot_property_old(filename{n},'Colorbar','off' , 'time' , n*time_step );
   if ( ~isempty(sea_level) )
      add_sea_level_plot( sea_level , 'time' , n*time_step );
   end
   M(n)=getframe( gcf );
end


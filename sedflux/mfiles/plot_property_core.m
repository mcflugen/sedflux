function h=plot_property_core( filename , position , varargin )
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

valid_args = { 'overlay'  , 'logical' , false ; ...
               'burial'   , 'logical' , true ; ...
               'time'     , 'logical' , false ; ...
               'sand'     , 'double' , [] };

values = parse_varargin( valid_args , varargin );

overlay      = values{strmatch( 'overlay' , {valid_args{:,1}} , 'exact' )};
burial_depth = values{strmatch( 'burial'  , {valid_args{:,1}} , 'exact' )};
with_time    = values{strmatch( 'time'    , {valid_args{:,1}} , 'exact' )};
sand_val     = values{strmatch( 'sand'    , {valid_args{:,1}} , 'exact' )};


if ( with_time )
   [path , name , ext , ver ] = fileparts( filename );
   time_file = [name '.age'];

   while ( isempty( dir( time_file ) ) & ~strcmp( time_file, 'q') )
      disp( ['Can not find age file, ' time_file] );
      time_file = input( 'Please specify and age file (q to quit): ' , 's' );
   end

   if ( strcmp( time_file , 'q' ) )
      error( 'No valid age file.' );
   end

   [ time depth property_id ] = get_property_core( time_file , position , ...
                                                   'BURIAL' , burial_depth );
end

if ( overlay )
   burial_depth = false;
end

[ core depth property_id ] = get_property_core( filename , position , ...
                                                'BURIAL' , burial_depth );

if ( with_time )
   depth = time;
end

[property_name , units] = get_property_full_name( property_id );

for i=1:length(core)
   c_order = get(gca,'colororder');
   if ( overlay )
      hold on
      [xx,yy] = stairs( depth{i} , core{i}+position(i)-mean(core{i}) );
%      plot( [ position(i) position(i) ] , [ min(depth{i}) max(depth{i}) ] , ...
%            'k' )
      if ( ~isempty( sand_val ) )
         core_sand = core{i};
         core_sand(core{i}>sand_val) = sand_val;
         area( core_sand+position(i)-mean(core{i}) , depth{i} , ...
               'facecolor' , [1 1 .8] , 'edgecolor' , 'none' )
      end
      h(i) = plot( yy , xx , 'color' , c_order(i,:) );
   else
      [xx,yy] = stairs( depth{i} , core{i} );
      if ( ~isempty( sand_val ) )
         hold on
         core_sand = core{i};
         core_sand(core{i}>sand_val) = sand_val;
         area( core_sand , depth{i} , ...
               'facecolor' , [1 1 .8] , 'edgecolor' , 'none' )
      end
      h(i) = plot( yy , xx , 'color' , c_order(i,:) );
   end
   hold on
end

if ( ~with_time )
   set(gca,'YDir','reverse');
end

if ( ~overlay )
%   xlabel('Grain Size (\phi)');
   xlabel( [property_name ' (' units ')' ] );
end
if ( burial_depth )
   if ( with_time )
      ylabel('Time (years)' );
   else
      ylabel('Burial Depth (m)');
   end
else
   ylabel('Water Depth (m)');
end

% We need position to be a column vector.
position = shiftdim(position);

labels = num2str(position);
labels = [labels char(ones(size(labels,1),1)*' km')];
legend(h,labels)

if ( overlay )
   for i=1:length(core)
      [xx,yy] = stairs( [ min(depth{i}) max(depth{i}) ] , ...
                        [ position(i)   position(i)   ] );
      plot( yy , xx , 'k' );
   end
end

hold off


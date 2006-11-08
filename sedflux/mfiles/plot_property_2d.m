function plot_property( filename , varargin )
% PLOT_PROPERTY   Plot a sedflux property file.
%
% PLOT_PROPERTY( FILENAME ) - plots the sedflux property file
% FILENAME as an image in the current axis.
%
% FILENAME can be followed by parameter/value pairs to specify
% additional properties of the image.  currently supported 
% parameters are:
%    'colorbar' - specify if a colorbar should or should not be
%                 be drawn with the image ('on' or 'off').  
%    'skip'     - specify the number of bins to skip when reading
%                 the image from a file.
%    'rescale'  - specify a range to scale the data between.  the
%                 next argument is a vector giving the new minimum
%                 and maximum.
%    'nofloor'  - remove the initial bathymetry.
%
% the property that is plotted is defined within the property file
% by sedflux.  currently supported sedflux properties are:
%    * bulk density
%    * grain size
%    * facies
%    * age
%    * reflectance
%    * void ratio
%    * min void ratio
%    * viscosity
%    * friction angle
%    * permeability
%    * porosity
%    * relative density
%    * excess porewater pressure
%    * degree of consolidation
%

% Set the font sizes for the labels.
% This set is good for figures.
   title_font_size = 16;
   label_font_size = 14;
   axis_font_size = 12;

   nofloor = 0;
   rescale = 0;
   clim = [];
   CBAR=1;
   time = [];
   read_args = {};
   i = 0;

   types=strvcat('GRAIN_SIZE','BULK_DENSITY','AGE','POROSITY','FACIES');

%%%
%%% Parse the input arguments.
%%%
   while i < nargin-1
      i = i + 1;
      arg = varargin{i};
      if ( i+1 <= nargin-1 )
         next_arg = varargin{i+1};
      else
         next_arg = [];
      end
      if ischar(arg)
         if ( strmatch(upper(arg),'SKIP') )
            read_args = { read_args{:} , 'SKIP' , next_arg };
            i = i + 1;
         elseif ( strmatch(upper(arg),'ASCII' ) )
            read_args = { read_args{:} , 'ASCII' };
            i = i + 1;
         elseif ( strmatch(upper(arg),'FUNC' ) )
            read_args = { read_args{:} , 'FUNC' , next_arg };
            i = i + 1;
         elseif ( strmatch(upper(arg),'RESCALE' ) )
            rescale = 1;
            new_min = next_arg(1);
            new_max = next_arg(2);
            i = i+1;
         elseif ( strmatch(upper(arg),'CLIM') )
            clim = next_arg;
            i = i+1;
         elseif ( strmatch(upper(arg),'NOFLOOR' ) )
            nofloor = 1;
         elseif ( strmatch(upper(arg),'COLORBAR') )
            if ( strmatch(upper(next_arg),'OFF') )
               CBAR = 0;
            elseif( strmatch(upper(next_arg),'ON') )
               CBAR = 1;
            else
               error(['Error at argument number ' num2str(i) ': invalid colorbar flag']);
            end
            i = i + 1;
         elseif ( strmatch( upper(arg) , 'TIME' ) )
            if ( max(size(next_arg))==1 | isempty(next_arg) )
               time = next_arg;
            else
               error( 'Time must be a scalar value.' );
            end
            i = i + 1;
         end
      else
         error('Incorrect input argument.');
      end
   end

   [data header] = read_property(filename,read_args{:},'dim');

   if ( rescale )
      water = data==0;
      rock  = data==255;
      data  = double(data)*(header.max_val-header.min_val)/(253) + header.min_val;
      data(data>new_max) = new_max;
      data(data<new_min) = new_min;
      
      header.max_val = new_max;
      header.min_val = new_min;

      data  = uint8((data-header.min_val)/(header.max_val-header.min_val)*(253) + 1);

      data(water) = 0;
      data(rock) = 255;

   end
   
%   if ( type_number == 4 )
%      max_val=(header.max_val-2650.)/(1000-2650);
%      min_val=(header.min_val-2650.)/(1000-2650);
%   end

   eps = (header.max_val - header.min_val )/64;
   if ( isempty(clim) )
      clim = [header.min_val-eps header.max_val+eps];
   else
      clim = sort(clim);
   end

   [rows cols] = size(data);

   data = data+1;

   x_data = ([0:(header.n_x_cols-1)]*header.dx + header.ref_x)/1000.;
   y_data = ([0:(header.n_y_cols-1)]*header.dy + header.ref_y)/1000.;
   z_data = [(header.n_rows-1):-1:0]*header.cell_height + header.ref_z;

   sea_level = find(z_data>=header.sea_level);
   if ( ~isempty(sea_level) )
      sea_level = sea_level(end)
      is_air = zeros( size(data) , 'uint8' );
      is_air( 1:sea_level , : ) = 1;
      data( data==2 ) == 3;
      data( logical(is_air) & data==1 ) = 2;
   end

   z_data = -z_data;

   if ( nofloor )
      for i=1:size(data,2)
         col=data(:,i);
         col(col==1) = 0.;
         ind = find(col~=255);
         data(:,i) = 0.;
         data((end-length(ind)+1):end,i) = col(ind);
      end
   end

   if ( header.n_x_cols==1 )
      h=image(y_data,z_data,data);
   else
      h=image(y_data,z_data,data(:,:,50));
   end

   set(h,'tag','sedflux-2d-image');

   hold on
   plot( get(gca,'xlim') , -header.sea_level*ones(1,2) , 'k' );
   hold off

   h=gca;

   [property_name,units] = get_property_full_name(header.property);

   title( ['Cross-Section of ' property_name ] );

   set(h,'FontSize',axis_font_size);
   set(h,'ydir','reverse');

   xlabel('Distance (km)','FontSize',label_font_size);
   ylabel('Depth (m)','FontSize',label_font_size);
   
   map = get_property_colormap( header.property );
   set(gcf,'ColorMap',map);

%%%
%%% Draw the color scale
%%%
   if ( CBAR ) 

      % If this already is a sedflux-2d-image and there is a colorbar, then
      % there is no need to change the height of the image as it should
      % already have been done.
      if ( isempty( findobj( gcf , 'Tag' , 'Colorbar' )  ) )
         pos = get(h,'position');
         set(h,'position',[ pos(1) (pos(2)+pos(4)*.3) pos(3) .7*pos(4) ] );
      end

      bar_h=subplot(10,1,10);
      reset(bar_h)
      bar_h = colorbar(bar_h);
      axes(bar_h);
      child=findobj(bar_h,'type','image');
      if ( header.min_val == header.max_val )
      	header.min_val = header.min_val-1;
      	header.max_val = header.max_val+1;
      end
%      set(child,'XData',[1:255]/255*(header.max_val-header.min_val)+header.min_val);
      set(child,'xdata',clim)
   % Have to sort min and max in case of porosity where min -> 1 and max -> 0
      set(bar_h,'XLim',sort([header.min_val-eps header.max_val+eps]));
      xlabel( [property_name ' (' units ')'] );
   end
   
   path=pwd;
   set(gcf,'NumberTitle','off','Name',[ path '/' filename]);

   if ( ~isempty( time ) )
      text( .05 , .1 , [ 'Time = ' num2str(time) ' years' ] , ...
            'units' , 'normalized' )
   end


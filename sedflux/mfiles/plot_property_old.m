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
         elseif ( strmatch(upper(arg),'RESCALE' ) )
            rescale = 1;
            new_min = next_arg(1);
            new_max = next_arg(2);
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

   [data header] = read_property_old(filename,read_args{:});

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

   [rows cols] = size(data);

   x_data = ([0:(header.n_cols-1)]*header.cell_width + header.ref(1))/1000.;
   y_data = [(header.n_rows-1):-1:0]*header.cell_height + header.ref(2);

   sea_level = find(y_data>=header.sea_level);
   if ( ~isempty(sea_level) )
      data(data==1)=2;
      sea_level = sea_level(end);
      mask = data(1:sea_level,:);
      mask(mask==0) = 1;
      data(1:sea_level,:)=mask;
   end

   y_data = -y_data;

%   if ( CBAR )
%      axes( 'position' , [ .13 .377 .775 .549 ] );
%      subplot(1.35,1,1);
%   end
%   imagesc(x_data,y_data,data,[header.min_val header.max_val]);
   if ( nofloor )
      for i=1:size(data,2)
         col=data(:,i);
         col(col==1) = 0.;
         ind = find(col~=255);
         data(:,i) = 0.;
         data((end-length(ind)+1):end,i) = col(ind);
      end
   end
   h=image(x_data,y_data,data);
   set(h,'tag','sedflux-2d-image');
   h=gca;

   set(h,'FontSize',axis_font_size);
   set(h,'ydir','reverse');

%%%
%%% Make the water blue, and the bedrock some other color
%%%
   switch header.property
      case 1, map=colormap(sternspecial(256)); map=map(end:-1:1,:); % bulk density
      case 2, map=colormap(sandsiltclay(256));              % grain size
      case 3, map=colormap(sandsiltclay(256));              % plastic index
      case 4, map=colormap(sandsiltclay(256));              % facies
      case 5, map=colormap(banded( jet(256),32 ) );         % age
      case 6, map=colormap(seismic(256));                   % reflectance
      case 7, map=colormap(sternspecial(256));              % void ratio
      case 8, map=colormap(sternspecial(256));              % min void ratio
      case 9, map=colormap(sternspecial(256));              % viscosity
      case 10, map=colormap(sternspecial(256));             % friction angle
      case 11, map=colormap(sternspecial(256));             % pemeability
      case 12, map=colormap(sternspecial(256));             % porosity
      case 13, map=colormap(sternspecial(256));             % relative density
      case 14, map=colormap(sternspecial(256));             % mv
      case 15, map=colormap(sternspecial(256));             % cv
      case 16, map=colormap(sternspecial(256));             % shear strength
      case 17, map=colormap(sternspecial(256));             % cohesion
      case 18, map=colormap(sternspecial(256));             % excess porewater pressure
      case 19, map=colormap(sternspecial(256));             % degree of consolidation
      otherwise, map=colormap(sternspecial(256));
   end
   map(1,:)=[ .4 .4 1 ];
   map(2,:)=[ 1 1 1 ];
   map(end,:)=[ .7 .2 0 ];
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
      set(child,'XData',[1:255]/255*(header.max_val-header.min_val)+header.min_val);
   % Have to sort min and max in case of porosity where min -> 1 and max -> 0
      set(bar_h,'XLim',sort([header.min_val header.max_val]));
      switch header.property
         case 1, xlabel('Bulk Density (kg/m^{3})','FontSize',label_font_size);
         case 2, xlabel('Grain Size (\phi)','FontSize',label_font_size);
         case 3, xlabel('Plastic Index (-)','FontSize',label_font_size);
         case 4, xlabel('Facies (-)','FontSize',label_font_size);
         case 5, xlabel('Sediment Age (\it{years})','FontSize',label_font_size);
         case 6, xlabel('Reflectance (-)','FontSize',label_font_size);
         case 7, xlabel('Void Ratio (-)','FontSize',label_font_size);
         case 8, xlabel('Min Void Ratio (-)','FontSize',label_font_size);
         case 9, xlabel('Viscosity (m^2/s)','FontSize',label_font_size);
         case 10, xlabel('Friciton Angle (deg)','FontSize',label_font_size);
         case 11, xlabel('Permeabilty (m^2)','FontSize',label_font_size);
         case 12, xlabel('Porosity (-)','FontSize',label_font_size);
         case 13, xlabel('Relative Density (-)','FontSize',label_font_size);
         case 14, xlabel('M_v','FontSize',label_font_size);
         case 15, xlabel('C_v','FontSize',label_font_size);
         case 16, xlabel('Shear Strength','FontSize',label_font_size);
         case 17, xlabel('Cohesion (Pa)','FontSize',label_font_size);
         case 18, xlabel('Excess Porewater Pressure (Pa)','FontSize',label_font_size);
         case 19, xlabel('Degree of Consolidation (-)','FontSize',label_font_size);
         case 20, xlabel('Relative Pressure (-)','FontSize',label_font_size);
         case 21, xlabel('Fraction of sand (-)','FontSize',label_font_size);
         case 22, xlabel('Fraction of silt (-)','FontSize',label_font_size);
         case 23, xlabel('Fraction of clay (-)','FontSize',label_font_size);
         case 24, xlabel('Fraction of mud (-)','FontSize',label_font_size);
         case 25, xlabel('Grain Density (kg/m^3)','FontSize',label_font_size);
         case 26, xlabel('Minimum Pososity (-)','FontSize',label_font_size);
         case 27, xlabel('Maximum Pososity (-)','FontSize',label_font_size);
         case 28, xlabel('Hydraulic Conductivity (m/s)','FontSize',label_font_size);
         case 29, xlabel('Maximum Void Ratio (-)','FontSize',label_font_size);
         case 30, xlabel('Consolidation Coefficient (?)','FontSize',label_font_size);
         case 31, xlabel('Yield Strength (Pa)','FontSize',label_font_size);
         case 32, xlabel('Dynamic Viscosity (kg/m/s)','FontSize',label_font_size);
         otherwise, xlabel( 'Grain Size Fraction (-)','FontSize',label_font_size);
      end
   end
   
   path=pwd;
   set(gcf,'NumberTitle','off','Name',[ path '/' filename]);

   axes(h);
   xlabel('Distance (km)','FontSize',label_font_size);
   ylabel('Depth (m)','FontSize',label_font_size);
   
   switch header.property
      case 1, title('Cross-Section Showing Sediment Bulk Density','FontSize',title_font_size);
      case 2, title('Cross-Section Showing Sediment Grain Size','FontSize',title_font_size);
      case 3, title('Cross-Section Showing Sediment Grain Size','FontSize',title_font_size);
      case 4, title('Cross-Section Showing Sediment Facies','FontSize',title_font_size);
      case 5, title('Cross-Section Showing Sediment Age','FontSize',title_font_size);
      case 6, 
         title('Cross-Section Showing Sediment Reflectance','FontSize',title_font_size);
         colormap(seismic);
         ylabel('Two-way travel time (\mu seconds)','FontSize',title_font_size);
      case 7, title('Cross-Section Showing Sediment Void Ratio','FontSize',title_font_size);
      case 8, title('Cross-Section Showing Minimum Void Ratio','FontSize',title_font_size);
      case 9, title('Cross-Section Showing Viscosity','FontSize',title_font_size);
      case 10,  title('Cross-Section Showing Friction Angle','FontSize',title_font_size);
      case 11, title('Cross-Section Showing Permeability','FontSize',title_font_size);
      case 12, title('Cross-Section Showing Porosity','FontSize',title_font_size);
      case 13, title('Cross-Section Showing Relative Density','FontSize',title_font_size);
      case 14, title('Cross-Section Showing M_v','FontSize',title_font_size);
      case 15, title('Cross-Section Showing C_v','FontSize',title_font_size);
      case 16, title('Cross-Section Showing Shear Strength','FontSize',title_font_size);
      case 17, title('Cross-Section Showing Cohesion','FontSize',title_font_size);
      case 18, title('Cross-Section Showing Excess Porewater Pressure','FontSize',title_font_size);
      case 19, title('Cross-Section Showing Degree of Consolidation','FontSize',title_font_size);
      case 20, title('Cross-Section Showing Relative Pressure','FontSize',title_font_size);
      case 21, title('Cross-Section Showing Fraction of Sand','FontSize',title_font_size);
      case 22, title('Cross-Section Showing Fraction of Silt','FontSize',title_font_size);
      case 23, title('Cross-Section Showing Fraction of Clay','FontSize',title_font_size);
      case 24, title('Cross-Section Showing Fraction of Mud','FontSize',title_font_size);
      otherwise,
          title(['Cross-Section Showing Fraction of Grain Size ' num2str(header.property-100,1)],'FontSize',title_font_size);
   end

   if ( ~isempty( time ) )
      text( .05 , .1 , [ 'Time = ' num2str(time) ' years' ] , ...
            'units' , 'normalized' )
   end

   set(h,'Tag',filename);
   UserData.xscale    = header.cell_width;
   UserData.yscale    = header.cell_height;
   UserData.water_val = header.water_val;
   UserData.rock_val  = header.rock_val;
   UserData.minval    = header.min_val;
   UserData.maxval    = header.max_val;
   set(h,'UserData',UserData);
   
%   UISedFluxTools;   
   

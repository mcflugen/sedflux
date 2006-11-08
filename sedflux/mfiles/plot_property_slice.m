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

valid_args = { 'xslice'     , 'double' , []   ; ...
               'yslice'     , 'double' , []   ; ...
               'zslice'     , 'double' , []   ; ...
               'colorbar'   , 'char'   , 'true' ; ...
               'nofloor'    , 'char'   , 'false' ; ...
               'rescale'    , 'char'   , 'false' ; ...
               'time'       , 'double' , [] };

values = parse_varargin( valid_args , varargin );

x_slice    = values{strmatch( 'xslice'   , {valid_args{:,1}} , 'exact' )};
y_slice    = values{strmatch( 'yslice'   , {valid_args{:,1}} , 'exact' )};
z_slice    = values{strmatch( 'zslice'   , {valid_args{:,1}} , 'exact' )};
cbar_is_on = values{strmatch( 'colorbar' , {valid_args{:,1}} , 'exact' )};
no_floor   = values{strmatch( 'nofloor'  , {valid_args{:,1}} , 'exact' )};
rescale    = values{strmatch( 'rescale'  , {valid_args{:,1}} , 'exact' )};
time       = values{strmatch( 'time'     , {valid_args{:,1}} , 'exact' )};

% Set the font sizes for the labels.
% This set is good for figures.
   title_font_size = 16;
   label_font_size = 14;
   axis_font_size  = 12;

   fid = fopen( filename , 'r' );
   hdr = read_property_header_cube( fid );
   fclose( fid );

   if ( ~isempty(x_slice) )
      columns = sub2ind( [hdr.n_y_cols hdr.n_x_cols] , ...
                         1:hdr.n_y_cols              , ...
                         ones(1,hdr.n_y_cols)*x_slice );
      x_data = ([1:hdr.n_y_cols]*hdr.cell_y_width + hdr.ref(2))/1000;
   elseif ( ~isempty(y_slice) )
      columns = sub2ind( [hdr.n_y_cols hdr.n_x_cols] , ...
                         ones(1,hdr.n_x_cols)*y_slice , ...
                         1:hdr.n_x_cols              );
      x_data = ([1:hdr.n_x_cols]*hdr.cell_x_width + hdr.ref(1))/1000;
   end

   if ( isempty( z_slice ) )
      [data header] = read_property_cube(filename,'COLS',columns);
   else
      [data header] = read_property_cube( filename );
      data = reshape( data , hdr.n_rows , hdr.n_y_cols , hdr.n_x_cols );
      data = squeeze(data( z_slice , : , : ));
      x_data = ([1:hdr.n_x_cols]*hdr.cell_x_width + hdr.ref(1))/1000;
      y_data = ([1:hdr.n_y_cols]*hdr.cell_y_width + hdr.ref(2))/1000;
   end

   if ( strcmpi( rescale , 'true' ) )
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

%   x_data = ([0:(header.n_cols-1)]*header.cell_width + header.ref(1))/1000.;
   z_data = [(header.n_rows-1):-1:0]*header.cell_height + header.ref(3);

   sea_level = find(z_data>=header.sea_level);
   if ( ~isempty(sea_level) )
      data(data==1)=2;
      sea_level = sea_level(end);
      mask = data(1:sea_level,:);
      mask(mask==0) = 1;
      data(1:sea_level,:)=mask;
   end

   z_data = -z_data;

%   if ( strcmpi( cbar_is_on , 'true' ) )
%      subplot(1.35,1,1);
%   end

%   imagesc(x_data,y_data,data,[header.min_val header.max_val]);

   if ( strcmpi( no_floor , 'true' ) )
      for i=1:size(data,2)
         col=data(:,i);
         col(col==1) = 0.;
         ind = find(col~=255);
         data(:,i) = 0.;
         data((end-length(ind)+1):end,i) = col(ind);
      end
   end

   if ( isempty(z_slice) )
      h=image(x_data,z_data,data);
   else
      h=image(x_data,y_data,data);
   end

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
   if ( strcmpi( cbar_is_on , 'true' ) ) 

      % If this already is a sedflux-2d-image and there is a colorbar, then
      % there is no need to change the height of the image as it should
      % already have been done.
      if ( isempty( findobj( gcf , 'Tag' , 'Colorbar' )  ) )
         pos = get(h,'position');
         set(h,'position',[ pos(1) (pos(2)+pos(4)*.3) pos(3) .7*pos(4) ] );
      end

      bar_h=subplot(10,1,10)
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
         case 5, xlabel('Sediment Age (years)','FontSize',label_font_size);
         case 6, xlabel('Reflectance (-)','FontSize',label_font_size);
         case 7, xlabel('Void Ratio (-)','FontSize',label_font_size);
         case 8, xlabel('Min Void Ratio (-)','FontSize',label_font_size);
         case 9, xlabel('Viscosity (m^2/s)','FontSize',label_font_size);
         case 10, xlabel('Friciton Angle (deg)','FontSize',label_font_size);
         case 11, xlabel('Permeabilty (m/s)','FontSize',label_font_size);
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
   UserData.xscale    = header.cell_x_width;
   UserData.yscale    = header.cell_y_width;
   UserData.zscale    = header.cell_height;
   UserData.water_val = header.water_val;
   UserData.rock_val  = header.rock_val;
   UserData.minval    = header.min_val;
   UserData.maxval    = header.max_val;
   set(h,'UserData',UserData);
   
%   UISedFluxTools;   
   

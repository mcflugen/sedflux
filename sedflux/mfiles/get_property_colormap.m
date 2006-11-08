function map = get_property_colormap( nick_name )

   switch nick_name
      case { 'grain' , 'density' , 'pi' , 'facies' }
	 map=colormap(sandsiltclay(256));
      case { 'age' }
	 map=colormap(banded( jet(256),32 ) );
      case { 'reflectance' , 'velocity' }
	 map=colormap(seismic(256));
      case { 'grain density' , 'max density' , 'grain in meters' }
	 map=colormap(sandsiltclay(256));
      case { 'sand' , 'silt' , 'clay' , 'mud' }
	 map=colormap( jet(256) );
      case { 'viscosity' , 'dynamic viscosity' , 'relative density' }
	 map=colormap( jet(256) );
      case { 'porosity' , 'porosity min' , 'porosity max' }
	 map=colormap( jet(256) );
      case { 'void ratio' , 'void ratio min' , 'void ratio max' }
	 map=colormap( jet(256) );
      case { 'permeability' , 'hydraulic conductivity' }
	 map=colormap( jet(256) );
      case { 'friction angle' , 'consolidation coefficient' , 'yield strength' }
	 map=colormap( sternspecial(256) );
      case { 'mv' , 'cv' , 'shear' , 'cohesion' , 'consolidation' }
	 map=colormap( sternspecial(256) );
      case { 'pressure' , 'excess pressure' , 'relative pressure' }
	 map=colormap( jet(256) );
      otherwise
	 map = colormap( jet(256) );
   end

%%%
%%% Make the water blue, and the bedrock some other color
%%%
   map(1,:)=[ .4 .4 1 ];
   map(2,:)=[ 1 1 1 ];
   map(end,:)=[ .3 .3 .25 ];



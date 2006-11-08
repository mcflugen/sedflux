function n = get_property_number( nick_name )

   nick_name = lower( nick_name );

   switch nick_name
      case 'density' , n = 1;
      case 'grain'   , n = 2;
      case 'pi'      , n = 3;
      case 'facies'  , n = 4;
      case 'age'     , n = 5;
      case 'reflectance' , n = 6;
      case 'void ratio' , n = 7;
      case 'void ratio min' , n = 8;
      case 'viscosity' , n = 9;
      case 'friction angle' , n = 10; 
      case 'permeability' , n = 11; 
      case 'porosity' , n = 12; 
      case 'relative density' , n = 13; 
      case 'mv' , n = 14; 
      case 'cv' , n = 15; 
      case 'shear' , n = 16; 
      case 'cohesion' , n = 17; 
      case 'excess pressure' , n = 18; 
      case 'consolidation' , n = 19; 
      case 'relative pressure' , n = 20; 
      case 'sand' , n = 21; 
      case 'silt' , n = 22; 
      case 'clay' , n = 23; 
      case 'mud' , n = 24; 
      case 'grain density' , n = 25; 
      case 'porosity min' , n = 26; 
      case 'porosity max' , n = 27; 
      case 'hydraulic conductivity' , n = 28; 
      case 'void ratio max' , n = 29; 
      case 'consolidation coefficient' , n = 30; 
      case 'yield strength' , n = 31; 
      case 'dynamic viscosity' , n = 32; 
      otherwise n = 0;
   end


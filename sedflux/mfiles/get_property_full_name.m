%% Get the full name of a property
% 
% \param nick_name The nickname of the propery. [string]
%
% \returns [full_name,units] The full name (as a string) and
%          and the units (as a string) of the property.
%
function [full_name,units] = get_property_full_name( nick_name )

   nick_name = lower( nick_name );

   switch nick_name
      case 'grain'   , full_name = 'Grain Size';   units = '\phi';
      case 'density' , full_name = 'Bulk Density'; units = 'kg/m^{3}';
      case 'pi'      , full_name = 'Plastic Index'; units = '-';
      case 'facies'  , full_name = 'Facies'; units = '';
      case 'age'     , full_name = 'Sediment Age'; units = 'years';
      case 'reflectance' , full_name = 'Reflectance'; units = '-';
      case 'grain density' , full_name = 'Grain Density'; units = '\phi';
      case 'sand' , full_name = 'Fraction Sand'; units = '-';
      case 'silt' , full_name = 'Fraction Silt'; units = '-';
      case 'clay' , full_name = 'Fraction Clay'; units = '-';
      case 'mud' , full_name = 'Fraction Mud'; units = '-';
      case 'viscosity' , full_name = 'Viscosity'; units = 'm^2/s';
      case 'dynamic viscosity' , full_name = 'Dynamic Viscosity'; units = 'kg/m/s';
      case 'relative density' , full_name = 'Relative density'; units = '-';
      case 'porosity' , full_name = 'Porosity'; units = '-';
      case 'porosity min' , full_name = 'Minimum Porosity'; units = '-';
      case 'porosity max' , full_name = 'Maximum Porosity'; units = '-';
      case 'void ratio' , full_name = 'Void Ratio'; units = '-';
      case 'void ratio min' , full_name = 'Minimum Void Ratio'; units = '-';
      case 'void ratio max' , full_name = 'Maximum Void Ratio'; units = '-';
      case 'permeability' , full_name = 'Permeability'; units = 'm^{2}';
      case 'hydraulic conductivity' , full_name = 'Hydraulic Conductivity'; units = 'm/s';
      case 'friction angle' , full_name = 'Friction Angle'; units = '\circ';
      case 'consolidation coefficient' , full_name = 'Consolidation Coefficient'; units = '-';
      case 'yield strength' , full_name = 'Yield Strength'; units = 'Pa';
      case 'mv' , full_name = 'mv'; units = '';
      case 'cv' , full_name = 'cv'; units = '';
      case 'shear' , full_name = 'Shear Strength'; units = 'Pa';
      case 'cohesion' , full_name = 'Cohesion'; units = 'Pa';
      case 'consolidation' , full_name = 'Consolidation'; units = '';
      case 'pressure' , full_name = 'Pressure'; units = 'Pa';
      case 'excess pressure' , full_name = 'Excess Pressure'; units = 'Pa';
      case 'relative pressure' , full_name = 'Relative Pressure'; units = 'Pa';
      case 'fraction',  full_name = 'Fraction of Grain Size'; units = '-';
      case 'slope', full_name = 'Seafloor Slope'; units = 'Gradient';
      case 'depth', full_name = 'Water Depth'; units = 'm';
      case 'elevation', full_name = 'Topographic Elevation'; units = 'm';
      case 'thickness', full_name = 'Sediment Thickness'; units = 'm';
      case 'basement', full_name = 'Elevation of Basement'; units = 'm';
      otherwise
         full_name = 'Unknown Property'; units = '?';
   end


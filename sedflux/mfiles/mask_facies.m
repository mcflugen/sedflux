function facies = mask_facies( facies , facies_name )

facies_name = upper(facies_name);

nothing_mask     = 0;
bedload_mask     = 1;
plume_mask       = 2;
debris_flow_mask = 4;
turbidite_mask   = 8;
diffused_mask    = 16;
river_mask       = 32;
wave_mask        = 64;

if ( strcmp( facies_name , 'NOTHING' ) )
   mask = nothing_mask;
elseif( strcmp( facies_name , 'BEDLOAD' ) )
   mask = bedload_mask;
elseif( strcmp( facies_name , 'PLUME' ) )
   mask = plume_mask;
elseif( strcmp( facies_name , 'DEBRIS FLOW' ) )
   mask = debris_flow_mask;
elseif( strcmp( facies_name , 'TURBIDITE' ) )
   mask = turbidite_mask;
elseif( strcmp( facies_name , 'DIFFUSED' ) )
   mask = diffused_mask;
elseif( strcmp( facies_name , 'RIVER' ) )
   mask = river_mask;
elseif( strcmp( facies_name , 'WAVE' ) )
   mask = wave_mask;
else
   error( 'Unknown facies name.' )
end

facies(isnan(facies)) = 0;
facies = bitand( facies , mask );


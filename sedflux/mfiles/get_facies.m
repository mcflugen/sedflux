function facies = get_facies( varargin )

filename = [];

if ( ~isempty( varargin ) )
   if ( ischar( varargin{1} ) )
      filename = varargin{1};
   else
      ax = varargin{1};
   end
else
   ax = gca;
end

n_facies = 7;

facies_mask_nothing     = 0;
facies_mask_bedload     = bitshift( 1 , 0 );
facies_mask_plume       = bitshift( 1 , 1 );
facies_mask_debris_flow = bitshift( 1 , 2 );
facies_mask_turbidite   = bitshift( 1 , 3 );
facies_mask_diffused    = bitshift( 1 , 4 );
facies_mask_river       = bitshift( 1 , 5 );
facies_mask_wave        = bitshift( 1 , 6 );

if ( ~isempty(filename) )
   [data header] = read_property(filename);
else
   h = get(ax,'child');
   h = h(strmatch( 'image' , get(h,'type') ));

   data = get(h,'cdata');
end
data(data==255) = 0;

facies_mask = { ones(size(data))*facies_mask_bedload     , ...
                ones(size(data))*facies_mask_plume       , ...
                ones(size(data))*facies_mask_debris_flow , ...
                ones(size(data))*facies_mask_turbidite   , ...
                ones(size(data))*facies_mask_diffused    , ...
                ones(size(data))*facies_mask_river       , ...
                ones(size(data))*facies_mask_wave};

for i=1:n_facies
   facies{i} = bitand( data , uint8(facies_mask{i}) );
end




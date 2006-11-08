function [core,depth,property] = get_property_core( filename , position , varargin )
% GET_PROPERTY_CORE   Get a core from a sedflux property file.
%
% GET_PROPERTY_CORE( filename , position )
%
% SEE ALSO 

valid_args = { 'burial'   , 'logical' , true };

values = parse_varargin( valid_args , varargin );

burial_depth = values{strmatch( 'burial'  , {valid_args{:,1}} , 'exact' )};

fid = fopen( filename , 'r' );

header = read_property_header( fid );

%[data header] = read_property( filename , 'dim' );

%%%
%%% Convert position where core will be taken, from km to bin number.
%%%

if ( islogical( position ) )

   data     = read_property( filename , ...
                             'mask'   , position , ...
                             'dim'    , true     , ...
                             'water'  , nan      , ...
                             'rock'   , nan );

else

   position = round( position*1000/header.dy );
   disp( [ 'reading column ' num2str( position ) ] );

   data     = read_property( filename , ...
                             'col'    , position , ...
                             'dim'    , true     , ...
                             'water'  , nan      , ...
                             'rock'   , nan );
end

for i=1:size(data,2)
   depth{i} = -([(header.n_rows-1):-1:0]*header.cell_height + header.ref_z );
   core{i}  = data(:,i);

   depth{i}( isnan(core{i}) ) = [];
   core{i}(isnan(core{i})) = [];

   y_max = size(core{i},1);
   if ( burial_depth )
      depth{i} = depth{i}-min(depth{i});
   end
%      depth{i}=[0:(y_max-1)]*header.cell_height;
end

property = header.property;


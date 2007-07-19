%% Parse a series of parameter/value pairs
%
% Parse the varargin parameter from a matlab function as a
% series of parameter/value pairs.
%
% \param valid_parameters   A cell array that defines the valid
%                           parameters
% \param input_params       Names of the input parameters
%
%

function value = parse_varargin( valid_parameters , input_params )

param_name       = {valid_parameters{:,1}};
param_class_name = {valid_parameters{:,2}};
param_default    = {valid_parameters{:,3}};

value = param_default;

i = 1;
while i<=length(input_params)

   this_arg = input_params{i};
   if ( i~=length(input_params) )
      next_arg = input_params{i+1};
   else
      next_arg = [];
   end

   if ( ischar( this_arg ) )
      param_no = strmatch( lower(this_arg) , lower(param_name) );
      if ( length(param_no)>1 )
         param_no = strmatch( lower(this_arg) , lower(param_name) , 'exact' );
      end

      if ( isempty(param_no) )
         error( ['Unknown parameter: ' this_arg] );
      end
      if ( isa( next_arg , param_class_name{param_no} ) )
         value{param_no} = next_arg;
      else
         error( [ 'Value for parameter, ' this_arg ...
                  ' is not of type ' param_class_name{param_no} ]  );
      end
      i = i+1;
   end

   i = i+1;

end

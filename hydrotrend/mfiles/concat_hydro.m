function x = concat_hydro( file_filter , varargin )

   valid_args = { 'format' , 'char' , 'native' };

   values = parse_varargin( valid_args , varargin );

   format = values{ strmatch( 'format' , {valid_args{:,1}} , 'exact' )};

filename = [];
if ( iscell( file_filter ) )
   for i=1:length(file_filter)
      file = dir(file_filter{i});
filename{end+1} = file_filter{i};
%      for n=1:size(file,1)
%         filename{end+1} = file(n).name
%      end
   end
%   filename = sort(filename);
else
   file = dir(file_filter);

   for n=1:length(file)
      filename{n} = file(n).name;
   end
   filename = sort(filename);
end

disp( ['Found ' num2str(length(filename)) ' files'] );

x = [];
for i=1:length( filename )
   disp( ['Reading file: ' filename{i}]);
   h = read_hydro( filename{i} , 'format' , format );
size(h)
   x = [x cell2mat(h)];
end

x = mat2cell( x , ones( size(x,1) , 1 ) );


function [log_data , column_label] = parse_log_file( file )

fid = fopen( file , 'r' );

for i=1:3
   fgets(fid);
end

column_label = {};
log_data     = {};

s = fgets( fid );
while ( s>0 )

   i = findstr( s , ':' );
   if ( length(i)==2 )
      [tok   , rem] = strtok( s   , ':' );
      [label , rem] = strtok( rem , ':' );
      [value , rem] = strtok( rem , ':' );

      label = regexprep( label , '^\s*|\s*$' , '' );
      value = sscanf( value , '%f' , 1 );

      column_no = strmatch( label , column_label , 'exact' );
      if ( isempty( column_no )  )
         column_no = length( column_label )+1;
         column_label = { column_label{:} label};
         log_data     = { log_data{:} value };
         disp( ['Found record name: ' label] );
      else
         log_data{column_no} = [ log_data{column_no} value ];
      end

   end
   s = fgets( fid );

end

%for i=1:length(log_data)
%   log_data{i}(1:2:end) = [];
%end

fclose( fid );

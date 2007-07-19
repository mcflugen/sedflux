%% Parse the header information in a sedflux property file
%
% A series of parameter/value pairs that make up the header
% of a property file is parsed
%
% \param fid   File id of the sedflux property file
%
% \return A structure of labels and values that contains
%         strings of the header information
%
function hdr=parse_sedflux_header( fid )

   str = fgets( fid );
   if ( strcmp( str , '--- header ---' ) )
      error( 'This does not appear to be a sedflux header.' );
   end

   hdr.l = '';
   hdr.v = '';

   s = fgetl( fid );
   while( s>0 & ~strcmp(s,'--- data ---') )

      i = findstr( s , ':' );
      if ( length(i)==1 )
         [label , rem] = strtok( s   , ':' );
         [value , rem] = strtok( rem , ':' );

         label = regexprep( label , '^\s*|\s*$' , '' );
         value = regexprep( value , '^\s*|\s*$' , '' );
%         value = sscanf( value , '%f' , 1 );

         this_rec.l = label;
         this_rec.v = value;


         if ( isempty( strmatch( label , {hdr.l} , 'exact' ) ) )
            hdr = [ hdr ; this_rec ];
         else
            disp( ['Repeated label in header: ' this_rec.l] );
         end

      end
      s = fgetl( fid );

   end

hdr(1) = [];

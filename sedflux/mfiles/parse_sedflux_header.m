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

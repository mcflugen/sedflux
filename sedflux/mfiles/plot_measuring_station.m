function ax = plot_measuring_station( filename , varargin )
% PLOT_MEASURING_STATION   Plot a Wheeler type diagram from sedflux data.
%
% PLOT_MEASURING_STATION( filename )

clim = [];

if ( nargin>1 )
   for i=1:2:length(varargin)
      arg1 = varargin{i};
      if ( i+1<=length(varargin) )
         arg2 = varargin{i+1};
      end
      if ( ischar( arg1 ) )
         arg1 = upper(arg1);
         arg2 = upper(arg2);
         if ( strcmp(arg1, 'CLIM' ) )
            if ( length(arg2)==2 )
               clim = arg2;
            else
               error( 'Data limits must be vector of length 2.' );
            end
         else
            error( ['Unknown key: ' arg1] );
         end
      else
         error( 'Key must be a string.' );
      end
   end
end

[data,x,t,hdr] = read_measuring_station_cube( filename );

if ( strcmp( hdr.origin , 'river' ) )

   plot( t, data );
   xlabel( 'time (years)' )
   ylabel( hdr.parameter )
   set(gca,'ydir','norm')

else

   if ( ~isempty(clim) )
      imagesc( x(:,1)/1000 , t , data' , clim );
   else
      imagesc( x(:,1)/1000 , t , data' );
   end
   ax = gca;
   xlabel( 'Distance (km)' );
   ylabel( 'Time (years)' );
   h=colorbar( 'horiz' );
   axes( h );
   xlabel( hdr.parameter );
   axes(ax)
   set(gca,'ydir','norm')

end

ax = gca;

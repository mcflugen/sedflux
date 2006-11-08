function h = add_sea_level_plot( s , varargin )

if ( length( size(s) )~=2 | min( size(s) )~=2 )
   error( 'Sea level curve must be a 2D vector of 2 rows or columns.' )
end

if ( size(s,1)~=2 )
   s = shiftdim( s , 1 );
end

peer = [];
time = [];

if ( nargin>1 )

for i=1:length(varargin)
   arg1 = varargin{i};
   if ( i+1<=length(varargin) )
      arg2 = varargin{i+1};
   else
      arg2 = [];
   end
   if ( ischar(arg1) )
      arg1 = upper(arg1);
      if ( strcmp(arg1,'PEER') )
         if ( ishandle(arg2) & strcmp(get(arg2,'type'),'axes') )
            peer = arg2;
         else
            error( 'Second argument must be a scalar axes handle' );
         end
      elseif ( strcmp(arg1,'TIME') )
         if ( length(arg2)==1 )
            time = arg2;
         else
            error( 'Time argument must be a scalar.' );
         end
      else
         error( [ 'Invalid key. -- ' arg1 ] );
      end
   end
end

end

orig_gca = gca;

h = findall( gcf , 'tag' , 'sea level' );

if ( ~isempty(h) )
   delete(h);
end

if ( ~isempty(peer) )
      h_cb = colorbar( 'vert' , 'peer' , peer );
else
      h_cb = colorbar( 'vert' , 'peer' , gca );
end

h = get(h_cb,'child');
set( h , 'DeleteFcn' , [] )
delete( h )
axes( h_cb )
hold on
plot( s(2,:) , s(1,:) )
axis auto
ylabel( 'Time (years)' , 'FontSize' , 10 )
set(gca,'FontSize',10 )

if ( ~isempty(time) )
   sea_level = interp1( s(1,:) , s(2,:) , time );
   hold on
   plot( sea_level , time , '.b' , 'markersize' , 25 );
   hold off
end

set(gca,'xtick',get(gca,'xlim'))
set(gca,'tag','sea level')

if ( ~isempty(peer) )
   set( h_cb , 'ylim' , get( peer , 'ylim' ) );
   set( h_cb , 'ydir' , get( peer , 'ydir' ) );
end

axes(orig_gca)


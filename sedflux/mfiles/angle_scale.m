function h=angle_scale( ax , ang , varargin )
% ANGLE_SCALE   Add a scale to a sedflux property plot.
%
% ANGLE_SCALE( AX , ANG ) plots a box in the axis, AX with lines drawn
% at the angles specified by the vector, ANG.
%
% The AX, ANG arguments can be followed by parameter/value pairs to specify
% additional properties.  The additional properties include:
%    'size' - valid values are 'small', 'medium', 'large', or a scaler.  this
%             property specifies the size of the scale.  if a scaler is given,
%             the axis is scaled by this amount.  The default is 'small'.
%
% SEE ALSO PLOT_PROPERTY
%

possible_sizes = strvcat( 'small' , 'medium' , 'large' );
axis_size = 'medium';

i=0;
while ( i<length(varargin) )

   i = i+1;

   this_arg = varargin{i};
   if ( i<length(varargin) )
      next_arg = varargin{i+1};
   else
      next_arg = [];
   end

   if ( ischar( this_arg ) )
      if ( strcmp( lower(this_arg) , 'size' ) )
         i = i+1;
         if ( ischar( next_arg ) )
            switch strmatch( next_arg , possible_sizes )
               case 1,
                  axis_size = 'small'
               case 2,
                  axis_size = 'medium'
               case 3,
                  axis_size = 'large'
               otherwise,
                  error( [ 'invalid size value: ' next_arg ] );
            end
         else
            if ( max( size(next_arg) )==1 )
               axis_size = 'user';
               size_scale = next_arg;
            else
               error( 'axis size must be either a string of a scalar' )
            end
         end
      else
         error( [ 'invalid label: ' next_arg ] )
      end
   else
      error( 'argument must be a string' );
   end
end

if ( strcmp( axis_size , 'small' ) )
   size_scale = 1;
elseif ( strcmp( axis_size , 'medium' ) )
   size_scale = 1.5;
elseif ( strcmp( axis_size , 'large' ) )
   size_scale = 2;
elseif ( ~strcmp( axis_size , 'user' ) )
   error( 'invalid size' );
end

pos = get(gca,'position');
r = pos(3)/pos(4);

axis_position = [ [.7 .7] [.1 .1/r]*size_scale] ;

h = axes( 'position' , axis_position , 'buttondownfcn' , 'selectmoveresize' );

x_lim = get(ax,'xlim');
y_lim = get(ax,'ylim');

x_lim = [0 x_lim(2)-x_lim(1)]';
y_lim = [0 y_lim(2)-y_lim(1)]';

for i=1:length(ang)
   y(:,i)  = atan( ang(i)  * pi/180 ).*[0;1]*(x_lim(2)*1000/y_lim(2));
end

plot( [0 1] , y , 'y' , 'linewidth' , 2*size_scale );

for i=1:length(ang)
   if ( y(2,i) < 1 )
      text( 1 , y(2,i) , [num2str(ang(i)) ' ^0'] , 'fontsize' , 7*size_scale );
   else
      x = y_lim(2)/(x_lim(2)*1000)/atan( ang(i)*pi/180. );
      text( x , 1 , [num2str(ang(i)) ' ^0'] , 'fontsize' , 7*size_scale , 'vertical' , 'top' , 'horizontal' , 'center' );
   end
end

set( h , 'xtick' , [] , 'xticklabel' , [] )
set( h , 'ytick' , [] , 'yticklabel' , [] )
set( h , 'box'   , 'on' )
set( h , 'color' , 'none' );
set( h , 'xlim'  , [0 1] );
set( h , 'ylim'  , [0 1] );
set( h , 'ydir'  , 'rev' );
set( h , 'buttondownfcn' , 'moveaxis' );
set( h , 'tag' , 'legend' );



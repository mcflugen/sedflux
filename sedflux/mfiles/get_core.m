function [core,depth] = get_core( varargin )
% GET_CORE   get a core from a sedflux property image.
%
% GET_CORE - gets a core from the current axis.  the user
% defines the position of the core by clicking the mouse on
% the image.
%
% GET_CORE( AXIS ) - gets the core from the axis, AXIS.
%

if nargin==1
   this_axis = varargin{1};
else
   this_axis = gca;
end

pos = ginput(1);

h=get(this_axis,'child');
xdata=get(h,'xdata');
depth=get(h,'ydata');
cdata=get(h,'cdata');

[y,ind]=min(abs(xdata-pos(1)));
ind=ind(1);

core = double(cdata(:,ind));
core(core==0) = nan;
core(core==255) = nan;
core(core==1) = nan;

userdata = get( this_axis , 'userdata' );
core = core/256*(userdata.maxval-userdata.minval)+userdata.minval;


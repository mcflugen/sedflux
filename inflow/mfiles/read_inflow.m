function [DATA, varargout] = read_inflow(varargin)
% read_inflow  - Read turbidity current data generated with inflow

if nargin == 0
   f_name = 'hyper.out';
elseif nargin == 1
   f_name = varargin{1};
else
   error('Too many input arguments');
end

fid = fopen(f_name,'rb');

len = fread(fid,1,'int');
bathymetry = fread(fid,[len 3],'double');
DATA = fread(fid,[len inf],'double');

if nargout == 2
   varargout{1} = bathymetry;
end

fclose(fid);

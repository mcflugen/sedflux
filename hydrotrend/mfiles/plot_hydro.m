function plot_hydro( file , varargin )
% PLOT_HYDRO   Plot a hydrotrend output file.
%
% PLOT_HYDRO( FILENAME ) plot the data from a hydrotrend file.
%
% PLOT_HYDRO( FILENAME , LIM ) read only the data from FILENAME
% contained within LIM.  if LIM is a scalar, only the first LIM records are
% read.  alternatively, LIM can be a two element vector that gives the first
% and last record to read.
%
% SEE ALSO READ_HYDRO, HYDRO_INFO
%

   [data t]=read_hydro( file , varargin{:} );

   figure
   path=pwd;
   set(gcf,'NumberTitle','off','Name',[ path '/' file ]);
   
   subplot(5,1,1)
   stairs(t,data{1})
   ylabel('River Velocity (m/s)')
   
   subplot(5,1,2)
   stairs(t,data{2})
   ylabel('River Width (m)')
   
   subplot(5,1,3)
   stairs(t,data{3})
   ylabel('River Depth (m)')
   
   subplot(5,1,4)
   stairs(t,data{4})
   ylabel('Bed Load (kg/s)')
   
   subplot(5,1,5)
   stairs(t,data{5}')
   ylabel('Suspended Load (g/m^3)')
   
   xlabel('Time (years)')

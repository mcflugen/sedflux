function plot_hydro(varargin)
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

   nYears=inf;
   file_name='river.in';
   plotYears=[1 nYears];
   
   for i=1:nargin
      if ischar(varargin{i})
         file_name=varargin{i};
      else
         if ( max(size(varargin{i}))==1 )
            nYears=varargin{i};
            plotYears=[0 nYears];
         else
            plotYears=varargin{i};
         end
      end
   end

   [data nSeasons]=read_hydro(file_name,plotYears);
   time=([1:length(data{1})]+plotYears(1))/nSeasons;

   figure
   path=pwd;
   set(gcf,'NumberTitle','off','Name',[ path '/' file_name]);
   
   subplot(5,1,1)
   stairs(time,data{1})
   ylabel('River Velocity (m/s)')
   
   subplot(5,1,2)
   stairs(time,data{2})
   ylabel('River Width (m)')
   
   subplot(5,1,3)
   stairs(time,data{3})
   ylabel('River Depth (m)')
   
   subplot(5,1,4)
   stairs(time,data{4})
   ylabel('Bed Load (kg/s)')
   
   subplot(5,1,5)
   stairs(time,data{5}')
   ylabel('Suspended Load (g/m^3)')
   
   xlabel('Time (years)')

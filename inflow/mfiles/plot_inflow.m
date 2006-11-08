function varargout = plot_inflow(filename)
% plot_inflow  - Plot tubidity currents generated with INFLOW

   [DATA bathy] = read_inflow(filename);
   bathy(:,1)   = bathy(:,1)/1000;

   figure
   set(gcf,'Name','Fluid Properties')
   subplot(2,1,1)
   plot(bathy(:,1),DATA(:,1))
   legend('Flow Concentration')
   subplot(2,1,2)
   plot(bathy(:,1),DATA(:,[2 3]))
   legend('Flow Density','Water Density')

   figure
   set(gcf,'Name','Accelerations')
   subplot(2,1,1)
   plot(bathy(:,1),DATA(:,[5 6 7]))
   legend('Gravity','External Friction','Internal Friction')
   subplot(2,1,2)
   plot(bathy(:,1),DATA(:,4))
   legend('Total Acceleration')

   figure
   set(gcf,'Name','Node Properties')
   subplot(4,1,1)
   plot(bathy(:,1),DATA(:,8))
   legend('Entrainment')
   subplot(4,1,2)
   plot(bathy(:,1),DATA(:,9))
   legend('Flow Height')
   subplot(4,1,3)
   plot(bathy(:,1),DATA(:,10))
   legend('Flow Velocity')
   subplot(4,1,4)
   plot(bathy(:,1),DATA(:,11))
   legend('Richardson Number')
   
   figure
   set(gcf,'Name','Erosion/Deposition Rates')
   subplot(3,1,1)
   plot(bathy(:,1),DATA(:,12))
   legend('Erosion Rate')
   subplot(3,1,2)
   plot(bathy(:,1),DATA(:,13))
   legend('Deposition Rate')
   subplot(3,1,3)
   plot(bathy(:,1),bathy(:,2))
   legend('Bathymetry')


function [x,y,conc]=PlotPlume(file)
% PLOTPLUME - Plot a plume2 binary output file.

%%%
%%% Read the plume data
%%%
   [x, y, conc]=ReadPlume(file);
   x = x/1000;
   y = y/1000;

%%%
%%% This will be the range for all the plots
%%%
   range = [min(min(min(conc))) max(max(max(conc)))];

%%%
%%% Plot each of the grain sizes
%%%
   nGrains = size(conc,3);
   for i=1:nGrains
      subplot(nGrains,1.1,1+(i-1)*1.1)
      imagesc(y,x,conc(:,:,i)',range);
      set(gca,'YDir','normal');
   end
   last_ax = gca;

%%%
%%% Draw the colorbar
%%%
   subplot(1,10,10)
   colorbar(gca)
   c=[0:255]/255*(range(2)-range(1))+range(1);
   h=get(gca,'Children');
   set(h,'YData',c)
   set(gca,'YLim',range);

   axes( last_ax );

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%
%%% Function : READPLUME
%%%
%%% Read a plume2 binary output file
%%%
function [x, y, conc]=ReadPlume(file)

	fid = fopen(file);

	nx = fread(fid,1,'int32');
	ny = fread(fid,1,'int32');
	ng = fread(fid,1,'int32');
	if ( ng == 0 )
		ng = 1;
	end
	depth = fread(fid,1,'double');

	x = fread(fid,nx,'double');
	y = fread(fid,ny,'double');
	conc = zeros(ny,nx,ng);
   conc(1:end) = fread(fid,ny*nx*ng,'double');
   
	fclose(fid);


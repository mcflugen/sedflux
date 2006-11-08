% HYDRO_STARTUP   Add the hydro mfiles to PATH
% 
%  User specific information, supplementing 
%	/usr/local/matlab/toolbox/local/matlabrc.m

%  Add M-File directories to MATLABPATH

%%%
%%% sedflux M-files
%%%

path(path,'/home/plum/huttone/local/ew114/share/hydro_matlab_files');


%%%
%%% Set the default axis text size and font
%%%

set(0,'DefaultAxesFontSize', 10);
set(0,'DefaultTextFontSize', 12);
%set(0,'DefaultAxesFontName', 'times');
%set(0,'DefaultTextFontName', 'times');
set(0,'DefaultAxesColorOrder',[1 0 0; 0 0 1; 0 1 0; 1 1 0; 1 0 1; 0 1 1]);


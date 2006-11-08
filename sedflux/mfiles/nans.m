function A=nans(varargin)
% NANS   Nans array.
%    NANS(N) is an N-by-N matrix of ones.
%    NANS(M,N) or NANS([M,N]) is an M-by-N matrix of ones.
%    NANS(M,N,P,...) or NANS([M N P ...]) is an M-by-N-by-P-by-...
%    array of ones.
%    NANS(SIZE(A)) is the same size as A and all ones.
% 
%    See also EYE, ONES, ZEROS.
%

A = nan*ones(varargin{:});


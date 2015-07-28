function [pks,locs] = findpeaks(X,varargin)
%FINDPEAKS Find local peaks in data
%   PKS = FINDPEAKS(X) finds local peaks in the data vector X.
%
%   [PKS,LOCS]= FINDPEAKS(X) also returns the indices LOCS at which the
%   peaks occur.
%
%   [...] = FINDPEAKS(X,'MINPEAKHEIGHT',MPH) finds only those peaks that
%   are greater than MINPEAKHEIGHT MPH. Specifying a minimum peak height
%   may help in reducing the processing time. MPH is a real valued scalar.
%   The default value of MPH is -Inf.
%
%   [...] = FINDPEAKS(X,'MINPEAKDISTANCE',MPD) finds peaks that are at
%   least separated by MINPEAKDISTANCE MPD. MPD is a positive integer
%   valued scalar. This parameter may be specified to ignore smaller peaks
%   that may occur in close proximity to a large local peak. For example,
%   if a large local peak occurs at index N, then all smaller peaks in the
%   range (N-MPD, N+MPD) are ignored. If not specified, MPD is assigned a
%   value of one.
%
%   [...] = FINDPEAKS(X,'THRESHOLD',TH)finds peaks that are at least
%   greater than their neighbhors by the THRESHOLD TH. TH is real valued
%   scalar greater than or equal to zero. The default value of TH is zero.
%
%   [...] = FINDPEAKS(X,'NPEAKS',NP) specifies the number of peaks to be
%   found. NP is an integer greater than zero. When specified, the
%   procedure terminates once NP peaks are found and the NP peaks are
%   returned. If not specified, all peaks are returned.
%
%   [...] = FINDPEAKS(X,'SORTSTR',STR) specifies the direction of sorting
%   of peaks. STR can take values of 'ascend','descend' or 'none'. If not
%   specified, STR takes the value of 'none' and the peaks are returned in
%   the order of their occurrence.
%
%   See also DSPDATA/FINDPEAKS

%   Copyright 2007-2009 The MathWorks, Inc.
%   $Revision: 1.1.6.8 $  $Date: 2009/05/23 08:13:26 $

error(nargchk(1,11,nargin,'struct'));

validateattributes(X,{'numeric'},{'nonempty','real','vector'},...
    'findpeaks','X');

%#function dspopts.findpeaks
hopts = uddpvparse('dspopts.findpeaks',varargin{:});

Ph  = hopts.MinPeakHeight;
Pd  = hopts.MinPeakDistance;
Th  = hopts.Threshold;
Np  = hopts.NPeaks;
Str = hopts.SortStr;

pks = [];
locs = [];


M = numel(X);

if (M < 3)
    datamsgid = generatemsgid('emptyDataSet');
    error(datamsgid,'Data set must contain at least 3 samples.');
else
    Indx = find(X > Ph);
    if(isempty(Indx))
        mphmsgid = generatemsgid('largeMinPeakHeight');
        warning(mphmsgid,'Invalid MinPeakHeight. There are no data points greater than MinPeakHeight.');
    else
        % validate value of Pd and set default values for Pd and Np
        [Pd,Np] = setvalues(Pd,Np,M);
        if(Pd >= M)
            pdmsgid = generatemsgid('largeMinPeakDistance');
            error(pdmsgid,'Invalid MinPeakDistance. Set MinPeakDistance as an integer in the range between 1 and %s.',...
                num2str(M));
        else
            [pks,locs] =getpeaks(X,Indx,Pd,Th,Np);
        end
    end
end

if isempty(pks)
    npmsgid = generatemsgid('noPeaks');
    warning(npmsgid,'No peaks found.')
elseif(~strcmp(Str,'none'))
    [pks,s]  = sort(pks,Str);
    if(~isempty(locs))
        locs = locs(s);
    end
end

%--------------------------------------------------------------------------
function [Pd,Np] = setvalues(Pd,Np,L)

if ~isempty(Pd) && (~isnumeric(Pd) || ~isscalar(Pd) ||any(rem(Pd,1)) || (Pd < 1))
    Nmsgid = generatemsgid('invalidMinPeakDistance');
    error(Nmsgid,'MinPeakDistance should be an integer greater than 0.');
end

if(isempty(Pd))
    Pd = 1;
end

if(isempty(Np))
    Np = L;
end

%--------------------------------------------------------------------------
function [pks,locs] =getpeaks(Data,Indx,Pd,Th,Np)
% This function finds peaks in data set Data whose index set Indx is
% disjoint. Some data points were removed from the original set through
% preprocessing

m = 0;                  % counter for peaks found
L = numel(Indx);
M = numel(Data);

% Pre-allocate for speed
pks  = zeros(1,Np);
locs = zeros(1,Np);

endindx = M;      % last point in the index set

j = 0;
% First, the "Pd" neighbors, on either side, of the current data point are
% found. Then the current data point is compared with these neighbors to
% determine whether it is a peak.

while (j < L) && (m < Np)
    j = j+1;
    
    currentIdx = Indx(j);
    
    % leftmost neighbor index
    endL = max(1,currentIdx - Pd);
    
    % Update the leftmost neighbor index if there is a peak within "Pd"
    % neighbors of leftmost index
    if(m > 0)
        prevPeakBoundL = min([locs(m)+Pd, endindx-1]);
        if currentIdx < prevPeakBoundL
            k = find(Indx(j+1:end),1,'first');
            if ~isempty(k)
                j = j+k;
                currentIdx = Indx(j);
            else
                break;
            end
        end
    end
    
    % rightmost neighbor index
    endR = min(currentIdx + Pd,endindx);
    
    % create neighbor data set
    temp = Data(endL:endR);
    
    % set current data point to -Inf in the neighbor data set
    temp(currentIdx-endL+1) = -Inf;
    
    % Compare current data point with all neighbors
    if(all((Data(currentIdx) > temp+Th)) && (currentIdx ~=1)&& (currentIdx~=M))
        m = m+1;
        locs(m) = currentIdx;  % loctions of peaks
        pks(m)  = Data(currentIdx);  % peaks
    end
end

% return all peaks found
if m~=0
    locs = locs(locs > 0);
    pks  = pks(1:length(locs));
else
    locs = [];
    pks = [];
end


% [EOF]

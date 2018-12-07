function samples = loadFile(filename)
%  y = loadFile(filename)
%
% reads  complex sample files in 32 bit float
% 32 bits of float data for I followed by
% 32 bits of float data for Q
% for each complex sample
%

% Open File
fid = fopen(filename,'rb');
% read array of floats into memory
data = fread(fid,inf,'double');

samples = [];

% convert floats list into complex samples.
% odd (1) index is real component,
% even (2) index is imag component
for idx = 1:2:size(data)
    r = data(idx);
    i = data(idx+1);
    cval = complex(r,i);
    samples = [ samples; cval ];
end




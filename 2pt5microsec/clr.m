function cr = clr(str)
% define some nice colors

data = { ...
% reds
'matlab_red', [1, 0, 0];
'red', [163, 0, 0]/255;
% yellows
'matlab_yellow', [1, 1, 0];
% greens
'matlab_green', [0, 1, 0];
'light_green', [0, 205,	0]/255;
'forest_green', [0, 48, 19]/255;
'sea_green', [0, 89, 72]/255;
% cyans
'matlab_cyan', [0, 1, 1];
'cyan', [0, 255, 255]/255;
% blues
'matlab_blue', [0, 0, 1];
'blue', [21, 0, 163]/255;
'light_blue', [0, 53, 201]/255;
'dark_blue', [0, 17, 66]/255;
% violets
'matlab_magenta', [1 0 1];
'purple', [64, 0, 99]/255;
'dark_purple', [44, 0, 66]/255;
'maroon', [81, 3, 25]/255;
% others
'dark_brown', [86, 36, 0]/255;
'light_brown', [186, 89, 0]/255;
'black', [0, 0, 0]/255 ...
};

% array of matches, a 1 at index i means the i'th color is str
match_array = strcmp(str,data(:,1));
row = find(match_array);    % row of selected color
if isempty(row)
    error('that is not a valid color');
end
cr = cell2mat(data(row,2));

% code to show color
%{
fig = figure;
fig.Color = cell2mat(data(5,2));
%}

end
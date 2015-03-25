function ImageMaker(r, c, scaling)
%IMAGEMAKER Summary of this function goes here
%   Detailed explanation goes here
global array;
array = -1 * ones(r, c);
global CELL_WIDTH;
global BOUNDARY_WIDTH;
global SQUARE_WIDTH;
global handle;
global eth;
CELL_WIDTH = scaling + 3;
BOUNDARY_WIDTH = 3;
SQUARE_WIDTH = CELL_WIDTH - BOUNDARY_WIDTH;
img = ones(r * CELL_WIDTH + BOUNDARY_WIDTH, c * CELL_WIDTH + BOUNDARY_WIDTH);

fh = figure;
eth = uicontrol(fh,'Style','edit',...
    'String','Enter filename here.',...
    'Position',[30 10 130 20]);

pbh = uicontrol(fh,'Style','pushbutton','String','Save Image',...
    'Position',[170 10 130 20]);
set(pbh, 'Callback', @saveImage);

for i = 0:r-1
    ind1 = CELL_WIDTH * i + 1;
    img(ind1:ind1 + BOUNDARY_WIDTH - 1, :)  = 0;
end
img(end - BOUNDARY_WIDTH + 1:end, :) = 0;

for i = 0:c-1
    ind1 = CELL_WIDTH * i + 1;
    img(:, ind1:ind1 + BOUNDARY_WIDTH - 1)  = 0;
end
img(:, end - BOUNDARY_WIDTH + 1:end) = 0;



handle = imshow(img);
set(handle, 'buttondownfcn', @draw);

end

function draw(gcbo, eventdata, handles)
global array;
global handle;
global CELL_WIDTH;
global BOUNDARY_WIDTH;
global SQUARE_WIDTH;

 
point = get(gca, 'Currentpoint');
c = round(point(1, 1));
r = round(point(1, 2));

indR = floor(r / CELL_WIDTH) + 1;
indC = floor(c / CELL_WIDTH) + 1;

if array(indR, indC) == -1
    newVal = 1;
    colorVal = 0;
else
    newVal = -1;
    colorVal = 1;
end
array(indR, indC) = newVal;
cdata = get(handle, 'Cdata');
colorIndR = (indR - 1) * CELL_WIDTH + 1 + BOUNDARY_WIDTH;
colorIndC = (indC - 1) * CELL_WIDTH + 1 + BOUNDARY_WIDTH;
cdata(colorIndR:colorIndR + SQUARE_WIDTH - 1, ...
      colorIndC:colorIndC + SQUARE_WIDTH - 1) = colorVal;
set(handle, 'Cdata', cdata);
end

function saveImage(gcbo, eventdata, handles)
global array;
global eth;
filename = get(eth, 'String');
if isempty(filename)
    % Throw error saving display images
else
    dlmwrite(filename, array, ' ');
end
end

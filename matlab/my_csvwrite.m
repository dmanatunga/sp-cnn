function my_csvwrite(filename, raw)
if isnumeric(raw)
    csvwrite(filename, raw);
elseif iscell(raw)
    [r, c] = size(raw);
    fh = fopen(filename, 'w');
    for i = 1:r-1
        writeRow(fh, raw(i, :), c, true);
    end
    writeRow(fh, raw(end, :), c, false);
    fclose(fh);
end
end

function writeRow(fh, row, c, includeNewLine)
for i = 1:c-1
   writeElement(fh, row{i});
   fprintf(fh, ',');
end
writeElement(fh, row{c});

if includeNewLine
    fprintf(fh, '\n');
end
end

function writeElement(fh, element)
if ~isempty(element)
    switch class(element)
        case 'char'
            fprintf(fh, '%s', element);
        case 'double'
            if mod(element, 1) == 0
                fprintf(fh, '%d', element);
            else
                fprintf(fh, '%f', element);
            end
    end
end
end

function HoleFilling_sp_cnn_fixed_conv()

cnn_dim = [64, 64];
numUnits = [1 4];
interval = 16;

test_dir = '../../test_files/';
tests = {'lowerA', 'capitalA', 'vertLine', 'circle', 'rect', 'filledSquare', 'zero', 'seven', 'eight', 'nine'};
%tests = {'filledSquare'};
raw = cell(length(tests) + 1, 5);
raw(1, 1:5) = {'Test', 'Num CNN', 'Num Iters.' 'Total Conv Time', 'Virtual Conv Time'};
ind = 2;
for i = 1:length(tests)
    for numCNN = numUnits
        test_name = tests{i};
        disp(test_name);
        img_file = [test_dir, test_name, '_256x256.dlm'];
        runData = HoleFilling_sp_cnn_fixed_interval(test_name, img_file, cnn_dim, interval, numCNN );
        raw{ind, 1} = test_name;
        raw{ind, 2} = numCNN;
        raw{ind, 3} = runData.numIters;
        raw{ind, 4} = runData.tot_conv_time;
        raw{ind, 5} = runData.virt_conv_time;
        ind = ind + 1;
    end
end
my_csvwrite('fixed_interval_conv_results/results.csv', raw);
end

function [runData] = HoleFilling_sp_cnn_fixed_interval(test_name, img_file, cnn_dim, interval, numCNN)
img = dlmread(img_file);
dim = size(img);

[r, A, B, z, u, initState ] = HoleFilling_gene(img);

partitions = gen_partitions2D(dim, cnn_dim, 'row-major');

[runData, state, output ] = sp_cnn_conv('fixed-interval', cnn_dim, r, A, B, z, initState, u, ...
    partitions, interval, numCNN );
file_header = sprintf('fixed_interval_conv_results/%s_%d', test_name, numCNN);
dlmwrite([file_header '_state.dlm'], state, ' ');
dlmwrite([file_header '_output.dlm'], output, ' ');

end

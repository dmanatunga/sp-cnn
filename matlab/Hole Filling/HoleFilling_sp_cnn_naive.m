function HoleFilling_sp_cnn_naive()

cnn_dim = [128, 128];
test_dir = '../../test_files/';
tests = {'lowerA', 'capitalA', 'vertLine', 'circle', 'rect', 'filledSquare', 'zero', 'seven', 'eight', 'nine'};
%tests = {'filledSquare'};
raw = cell(length(tests) + 1, 3);
raw(1, 1:3) = {'Test', 'Total Conv Time', 'Virtual Conv Time'};
for i = 1:length(tests)
    test_name = tests{i};
    disp(test_name);
    img_file = [test_dir, test_name, '_1024x1024.dlm'];
    [tot_conv_time, virt_conv_time] = HoleFilling_naive(test_name, img_file, cnn_dim);
    raw{i+1, 1} = test_name;
    raw{i+1, 2} = tot_conv_time;
    raw{i+1, 3} = virt_conv_time;
end
my_csvwrite('naive_results/results.csv', raw);
end

function [tot_conv_time, virt_conv_time] = HoleFilling_naive(test_name, img_file, cnn_dim)
img = dlmread(img_file);
dim = size(img);

[r, A, B, z, u, initState ] = HoleFilling_gene(img);

partitions = gen_partitions2D(dim, cnn_dim, 'independent');

[tot_conv_time, virt_conv_time, state, output ] = sp_cnn_naive( cnn_dim, r, A, B, z, initState, u, ...
    partitions );

dlmwrite(['naive_results/', test_name, '_state.dlm'], state, ' ');
dlmwrite(['naive_results/', test_name, '_output.dlm'], output, ' ');                           
end
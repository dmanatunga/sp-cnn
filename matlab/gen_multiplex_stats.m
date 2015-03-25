function gen_multiplex_stats(img_file, gene, cnn_dim, N, dt, partition_type, results_dir)

img = dlmread(img_file);
dim = size(img);
[r, A, B, z, u, initState ] = gene(img);
partitions = gen_partitions2D(dim, cnn_dim, partition_type);

cnn_data = [];
prime_factors = factor(N);
max_divisor = prod(prime_factors(2:end));
divisors = max_divisor:-1:1;
interval_times = N ./ divisors(mod(N, divisors) == 0);
interval_times = [1, interval_times];

for i = 1:length(interval_times)
    cnn_N = interval_times(i);
    partition_N = N / cnn_N;
    
    fprintf('Running CNN Multiplex - (cnn_N=%d, partition_N=%d)\n', cnn_N, partition_N);
    samples = cnn2D_multiplex(cnn_dim, r, A, B, z, initState, u, ...
        dt, cnn_N, partition_N, partitions, 'sample-iter');
    
    cnn_data = [cnn_data, struct('cnn_N', cnn_N, 'partition_N', partition_N, 'samples', samples)];
end
sample_list = 1:N;

fprintf('Running ideal CNN...\n');
ideal_data = cnn2D( dim, r, A, B, z, initState, u, dt, N, 'sample', sample_list);
final_output = ideal_data(end).output;

ROWS_PER_ELEM = 5;
error_data = cell((length(cnn_data) + 1) * ROWS_PER_ELEM, length(sample_list) + 1);

stable_data = cell((length(cnn_data)+2), 3);
stable_data{1, 1} = 'Interval Time';
stable_data{1, 2} = 'Stable Virtual (t)';
stable_data{1, 3} = 'Stable Iteration';

stable_data{2, 1} = 'Ideal';
stable_data{2, 2} = findConvergencePoint(final_output, ideal_data);
stable_data{2, 3} = stable_data{2, 2};

error_data{1, 1} = 'Ideal';
error_data{2, 1} = 'error-compared-to-final';
error_data(1, 2:end) = num2cell(sample_list);
for i = 1:length(ideal_data)
    error_data{2, i+1} = output_error(final_output, ideal_data(i).output);
end

fig = figure('units','normalized','outerposition',[0 0 1 1]);

g1 = subplot(3, 1, 1);
g2 = subplot(3, 1, 2);
g3 = subplot(3, 1, 3);

plot1_args = cell(1, (length(cnn_data) + 1) * 3);
plot1_legend = cell(1, (length(cnn_data) + 1));

plot1_args{end-2} = sample_list;
plot1_args{end-1} = cell2mat(error_data(2, 2:end));
plot1_args{end} = '-*';
plot1_legend{end} = 'Ideal-Case';

plot2_args = cell(1, length(cnn_data) * 3);
plot2_legend = cell(1, length(cnn_data));

plot3_args = cell(1, length(cnn_data) * 3);
plot3_legend = cell(1, length(cnn_data));

for i = 1:length(cnn_data)
    ind = i * ROWS_PER_ELEM + 1;
    error_data{ind, 1} = cnn_data(i).cnn_N;
    error_data{ind+1, 1} = 'avg-state-error';
    error_data{ind+2, 1} = 'output-error';
    error_data{ind+3, 1} = 'error-compared-to-final';
    samples = cnn_data(i).samples;
    iters = {samples.iter};
    error_data(ind, 2:length(iters) + 1) = iters;
    
    for j = 1:length(iters)
        iter = iters{j};
        ideal_iter = ideal_data(sample_list == iter);
        error_data{ind+1, j+1} = avg_error(ideal_iter.state, samples(j).state);
        error_data{ind+2, j+1} = output_error(ideal_iter.output, samples(j).output);
        error_data{ind+3, j+1} = output_error(final_output, samples(j).output);
    end
    
    [stablePoint, partition_iter] = findConvergencePoint(final_output, samples);
    stable_data{i + 2, 1} = cnn_data(i).cnn_N;
    stable_data{i + 2, 2} = stablePoint;
    stable_data{i + 2, 3} = partition_iter;
    
    sim_case = sprintf('Interval-Time = %d', cnn_data(i).cnn_N);
    x_vals = cell2mat(iters);
    plot1_args{(i - 1) * 3 + 1} = x_vals;
    plot1_args{(i - 1) * 3 + 2} = cell2mat(error_data(ind+3, 2:length(iters) + 1));
    plot1_args{(i - 1) * 3 + 3} = '-*';
    plot1_legend{i} = sim_case;
    
    plot2_args{(i - 1) * 3 + 1} = x_vals;
    plot2_args{(i - 1) * 3 + 2} = cell2mat(error_data(ind+2, 2:length(iters) + 1));
    plot2_args{(i - 1) * 3 + 3} = '-*';
    plot2_legend{i+1} = sim_case;
    
    plot3_args{(i - 1) * 3 + 1} = x_vals;
    plot3_args{(i - 1) * 3 + 2} = cell2mat(error_data(ind+1, 2:length(iters) + 1));
    plot3_args{(i - 1) * 3 + 3} = '-*';
    plot3_legend{i} = sim_case;
end
ind = find((img_file == '/') | (img_file == '\'), 1, 'last');
[name, ~] = strtok(img_file(ind+1:end), '.');

plot_name = name;
plot_name(plot_name == '_') = '-';

plot(g1, plot1_args{:});
title(g1, sprintf('Test %s - cnnDim [%d, %d] - (N=%d, dt=%d) - %s\nError Between Output(t) and Final Ideal-Output', plot_name, cnn_dim(1), cnn_dim(2), N, dt, partition_type));
xlabel(g1, 'Virtual Time (t)');
ylabel(g1, 'Number of Pixels Different');
legend(g1, plot1_legend{:});

plot(g2, plot2_args{:});
title(g2, 'Error Between Output(t) and Ideal-Output(t)');
xlabel(g2, 'Virtual Time (t)');
ylabel(g2, 'Number of Pixels Different');
legend(g2, plot2_legend{:});

plot(g3, plot3_args{:});
title(g3, 'Error Between State(t) and Ideal-State(t)');
xlabel(g3, 'Virtual Time (t)');
ylabel(g3, 'Average State Difference');
legend(g3, plot3_legend{:});

loc = sprintf('%s%s%s', results_dir, filesep, name);
if ~exist(loc, 'dir')
    mkdir(loc);
end
safe_type = partition_type;
safe_type(safe_type == '-') = '_';

filename = sprintf('%s%s%s_cnn%dx%d_%s', loc, filesep, name, cnn_dim(1), cnn_dim(2), safe_type);

export_fig(fig, [filename '_plot.png'], '-nocrop');
save([filename '.mat'], 'img_file', 'cnn_dim', 'N', 'dt', 'partition_type', 'results_dir', 'loc', ...
    'plot1_args', 'plot1_legend', 'plot2_args', 'plot2_legend', 'plot3_args', 'plot3_legend', ...
    'error_data', 'stable_data', 'sample_list', 'ideal_data', 'cnn_data', 'final_output');
close(fig);

my_csvwrite([filename, '_stats.csv'], error_data);
my_csvwrite([filename, '_stableData.csv'], stable_data);

end

function ret = avg_error(ideal, real)
ret = sum(sum(abs(ideal - real))) / numel(ideal);
end


function ret = output_error(ideal, real)
THRESHOLD = 0.001;
ret = sum(sum((abs(ideal - real) > THRESHOLD)));
end

function varargout = findConvergencePoint(ideal_output, samples)
if nargout == 2
    if output_error(ideal_output, samples(end).output) == 0
        ret = samples(end).iter;
        ret2 = samples(end).partition_iter;
        for i = length(samples)-1:-1:1
            if output_error(ideal_output, samples(i).output) ~= 0
                break;
            else
                ret = samples(i).iter;
                ret2 = samples(i).partition_iter;
            end
        end
    else
        ret = -1;
        ret2 = -1;
    end
    varargout{1} = ret;
    varargout{2} = ret2;
else
    if output_error(ideal_output, samples(end).output) == 0
        ret = samples(end).iter;
        for i = length(samples)-1:-1:1
            if output_error(ideal_output, samples(i).output) ~= 0
                break;
            else
                ret = samples(i).iter;
            end
        end
    else
        ret = -1;
    end
    varargout{1} = ret;
end
end
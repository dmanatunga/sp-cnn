function ret = gen_partitions2D(input_dim, cnn_dim, type)
switch type
    case 'independent'
        ret.partitions = row_major(input_dim, cnn_dim);
        ret.type = 'independent';
    case 'no-share-boundaries'
        ret.partitions = row_major(input_dim, cnn_dim);
        ret.type = 'no-share-boundaries';
    case 'row-major'
        [ret.partitions, ret.dim] = row_major(input_dim, cnn_dim);
        ret.type = 'dependent';
    case 'rev-row-major'
        seq = row_major(input_dim, cnn_dim);
        ret.partitions = seq(end:-1:1);
        ret.type = 'dependent';
    case 'col-major'
        ret.partitions = col_major(input_dim, cnn_dim);
        ret.type = 'dependent';
    case 'rev-col-major'
        seq = col_major(input_dim, cnn_dim);
        ret.partitions = seq(end:-1:1);
        ret.type = 'dependent';
    otherwise
        error('Invalid Sequence Type');
end
end

function [seq, dim] = row_major(input_dim, cnn_dim)
num_row_segs = ceil(input_dim(1) / cnn_dim(1));
num_col_segs = ceil(input_dim(2) / cnn_dim(2));
dim = [num_row_segs, num_col_segs];
num_partitions = num_row_segs * num_col_segs;
seq = cell(1, num_partitions);

row_inds = 1:cnn_dim(1):input_dim(1);
col_inds = 1:cnn_dim(2):input_dim(2);
partition_ind = 1;
for i = 1:num_row_segs
    for j = 1:num_col_segs
        seq{partition_ind} = [row_inds(i), col_inds(j)];
        partition_ind = partition_ind + 1;
    end
end
end

function seq = col_major(input_dim, cnn_dim)
num_row_segs = ceil(input_dim(1) / cnn_dim(1));
num_col_segs = ceil(input_dim(2) / cnn_dim(2));
num_partitions = num_row_segs * num_col_segs;
seq = cell(1, num_partitions);

row_inds = 1:cnn_dim(1):input_dim(1);
col_inds = 1:cnn_dim(2):input_dim(2);
partition_ind = 1;
for j = 1:num_col_segs
    for i = 1:num_row_segs
        seq{partition_ind} = [row_inds(i), col_inds(j)];
        partition_ind = partition_ind + 1;
    end
end
end
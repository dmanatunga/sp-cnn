testDir = '../../tests/';
dim_vals = 64;
cnn_ratios = [2 8];
partition_schemes = {'independent', 'row-major'};
root_dir = pwd;

for i = 1:length(cnn_ratios)
    cnn_ratio = cnn_ratios(i);
    app = 'Hole Filling';
    cd(app);
    fprintf('%s\n', app);
    HoleFilling_multiplexStats(testDir, dim_vals, cnn_ratio, partition_schemes);
    cd(root_dir);
    
    app = 'Shadow Creator';
    cd(app);
    fprintf('%s\n', app);
    ShadowCreator_multiplexStats(testDir, dim_vals, cnn_ratio, partition_schemes);
    cd(root_dir);
    
    app = 'Concentric Contour';
    cd(app);
    fprintf('%s\n', app);
    ConcentricContour_multiplexStats(testDir, dim_vals, cnn_ratio, partition_schemes);
    cd(root_dir);
    
    app = 'Connected Component';
    cd(app);
    fprintf('%s\n', app);
    ConnectedComponent_multiplexStats(testDir, dim_vals, cnn_ratio, partition_schemes);
    cd(root_dir);
    
    app = 'Corner Detection';
    cd(app);
    fprintf('%s\n', app);
    CornerDetection_multiplexStats(testDir, dim_vals, cnn_ratio, partition_schemes);
    cd(root_dir);
    
    app = 'Edge Detection';
    cd(app);
    fprintf('%s\n', app);
    EdgeDetection_multiplexStats(testDir, dim_vals, cnn_ratio, partition_schemes);
    cd(root_dir);
    
    app = 'Rotation Detector';
    cd(app);
    fprintf('%s\n', app);
    RotationDetector_multiplexStats(testDir, dim_vals, cnn_ratio, partition_schemes);
    cd(root_dir);
end

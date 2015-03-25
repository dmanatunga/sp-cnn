
#declare -a genes=("ConcentricContour" "ConnectedComponent" "CornerDetection" "EdgeDetection" "HoleFilling" "RotationDetector" "ShadowCreator")
declare -a genes=("HoleFilling")
declare -a test_files=("lowerA" "capitalA" "vertLine" "circle" "rect" "filledSquare" "zero" "seven" "eight" "nine")
declare -a numUnits=(1 4)

echo "Unit Test: Naive (Share) SP-CNN"
for gene in ${genes[@]} ; do
  for test in ${test_files[@]} ; do
    ./test-sp-cnn naive 128 128 ${gene} ../../test_files/${test}_1024x1024.dlm naive_results/${test}_output.dlm 
    echo ""
  done
done

echo "Unit Test: Naive (No-Share) SP-CNN"
for gene in ${genes[@]} ; do
  for test in ${test_files[@]} ; do
    ./test-sp-cnn naive-no-share 128 128 ${gene} ../../test_files/${test}_512x512.dlm naive_no_share_results/${test}_output.dlm
    echo ""
  done
done

echo "Unit Test: Fixed-Interval SP-CNN"
for gene in ${genes[@]} ; do
  for test in ${test_files[@]} ; do
    for numCNN in ${numUnits[@]} ; do
      ./test-sp-cnn fixed-interval 128 128 ${gene} ../../test_files/${test}_512x512.dlm 16 row-major ${numCNN} fixed_interval_results/${test}_${numCNN}_output.dlm 
      echo ""
    done
  done
done

echo "Unit Test: Early-Finish SP-CNN"
for gene in ${genes[@]} ; do
  for test in ${test_files[@]} ; do
    for numCNN in ${numUnits[@]} ; do
      ./test-sp-cnn early-finish 128 128 ${gene} ../../test_files/${test}_512x512.dlm 16 row-major ${numCNN} early_finish_results/${test}_${numCNN}_output.dlm 
      echo ""
    done
  done
done

echo "Unit Test: Fixed-Interval (fast) SP-CNN"
for gene in ${genes[@]} ; do
  for test in ${test_files[@]} ; do
    ./test-sp-cnn fixed-interval-fast 64 64 ${gene} ../../test_files/${test}_256x256.dlm 16 row-major 1 fixed_interval_fast_results/${test}_1_output.dlm
    echo ""
  done
done

echo "Unit Test: Early-Finish (fast) SP-CNN"
for gene in ${genes[@]} ; do
  for test in ${test_files[@]} ; do
    ./test-sp-cnn early-finish-fast 64 64 ${gene} ../../test_files/${test}_256x256.dlm 16 row-major 1 early_finish_fast_results/${test}_1_output.dlm 
    echo ""
  done
done

echo "Unit Test: Fixed-Interval (conv) SP-CNN"
for gene in ${genes[@]} ; do
  for test in ${test_files[@]} ; do
      for numCNN in ${numUnits[@]} ; do
        ./test-sp-cnn fixed-interval-conv 64 64 ${gene} ../../test_files/${test}_256x256.dlm 16 row-major ${numCNN} fixed_interval_conv_results/${test}_${numCNN}_output.dlm 
      echo ""
    done
  done
done

echo "Unit Test: Early-Finish (conv) SP-CNN"
for gene in ${genes[@]} ; do
  for test in ${test_files[@]} ; do
      for numCNN in ${numUnits[@]} ; do
        ./test-sp-cnn early-finish-conv 64 64 ${gene} ../../test_files/${test}_256x256.dlm 16 row-major ${numCNN} early_finish_conv_results/${test}_${numCNN}_output.dlm 
      echo ""
    done
  done
done


echo "Unit Test: Early-Finish (conv) SP-CNN"
for gene in ${genes[@]} ; do
  for test in ${test_files[@]} ; do
      for numCNN in ${numUnits[@]} ; do
        ./test-sp-cnn early-finish-conv 64 64 ${gene} ../../test_files/${test}_256x256.dlm 16 row-major ${numCNN} early_finish_conv_results/${test}_${numCNN}_output.dlm 
      echo ""
    done
  done
done

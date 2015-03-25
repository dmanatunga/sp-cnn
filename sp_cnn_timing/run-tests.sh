#declare -a genes=("ConcentricContour" "ConnectedComponent" "CornerDetection" "EdgeDetection" "HoleFilling" "RotationDetector" "ShadowCreator")
declare -a genes=("HoleFilling")
declare -a test_files=("lowerA" "capitalA" "vertLine" "circle" "rect" "filledSquare" "zero" "seven" "eight" "nine")
declare -a numUnits=(1 4)
declare -a shouldPrefetch=(2)

echo "Unit Test: SP-CNN Timing Fixed Interval"
for prefetch in ${shouldPrefetch[@]} ; do
  for numCNN in ${numUnits[@]} ; do
    for gene in ${genes[@]} ; do
      for test in ${test_files[@]} ; do
		echo "./sp-cnn-timing ${gene} ../test_files/${test}_1024x1024.dlm fixed-interval 128 128 128 "row-major" ../test_outputs/${gene}/${gene}_${test}_1024x1024.dlm --num_cnnP ${numCNN} --max_num_assigned_part ${prefetch}"
        ./sp-cnn-timing ${gene} ../test_files/${test}_1024x1024.dlm fixed-interval 128 128 128 "row-major" ../test_outputs/${gene}/${gene}_${test}_1024x1024.dlm --num_cnnP ${numCNN} --max_num_assigned_part ${prefetch}  
        echo ""
      done
    done
  done
done

echo "Unit Test: SP-CNN Timing Early-Finish"
for prefetch in ${shouldPrefetch[@]} ; do
  for numCNN in ${numUnits[@]} ; do
    for gene in ${genes[@]} ; do
      for test in ${test_files[@]} ; do
		echo "./sp-cnn-timing ${gene} ../test_files/${test}_1024x1024.dlm early-finish 128 128 128 "row-major" ../test_outputs/${gene}/${gene}_${test}_1024x1024.dlm --num_cnnP ${numCNN} --max_num_assigned_part ${prefetch}"
        ./sp-cnn-timing ${gene} ../test_files/${test}_1024x1024.dlm early-finish 128 128 128 "row-major" ../test_outputs/${gene}/${gene}_${test}_1024x1024.dlm --num_cnnP ${numCNN} --max_num_assigned_part ${prefetch}  
        echo ""
      done
    done
  done
done


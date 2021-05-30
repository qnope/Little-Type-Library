set -e
git branch -D documentation || true
git checkout -b documentation
sudo apt install doxygen ninja-build graphviz gcovr pcregrep
cmake -S . -B build -G "Ninja" -DLTL_BUILD_TESTS=ON -DLTL_TEST_CPP20=OFF -DLTL_GENERATE_DOCUMENTATION=ON -CMAKE_BUILD_TYPE=Debug
rm -rf docs || true
rm -rf coverage.json coverage.xml || true
cmake --build build > logs.out

cd build/Tests
ctest
cd ../..
gcovr -r . --xml-pretty > coverage.xml
COVERAGE=`gcovr -r . -s -e ".*Benchmark*" -e ".*gtest" | pcregrep -o1 "^lines: (.*)\%"`
echo "{\"coverage\": $COVERAGE}" > coverage.json

git config --global user.email "antoine.morrier@gmail.com"
git config --global user.name "Antoine MORRIER (Doc Workflow)"

mv docs/html docs/documentation
mkdir -p docs/coverage/
gcovr -r . --html --html-details -o docs/coverage/index.html -e ".*Benchmark*" -e ".*gtest"

git add docs
git add coverage.xml coverage.json
git commit --amend --no-edit > logs.out
git push -u origin documentation --force

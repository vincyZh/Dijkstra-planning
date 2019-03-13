# C++ 编译器 GNU GCC
CXX = g++

# O3优化(最大优化), C++11标准, 启用AVX指令集
# 启用AVX2指令集请添加选项 -mavx2
CXXFLAGS = -O3 -std=c++14 -mavx -fopenmp

TARGET = main

SOURCE = main.cpp

$(TARGET):
	$(CXX) $(CXXFLAGS) -I. $(SOURCE) -o $(TARGET)


.PHONY : clean

clean:
	@if [ -f $(TARGET) ]; then rm $(TARGET); fi

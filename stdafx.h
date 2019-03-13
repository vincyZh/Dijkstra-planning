//预编译头文件,将不需要经常修改的头文件放到此文件中,以加快编译速度

#ifndef _STDAFX_H_
#define _STDAFX_H_

//------------------------------Eigen Config Start------------------------------

//列优先改为行优先
#define EIGEN_DEFAULT_TO_ROW_MAJOR
/*
* 使用SIMD（单指令多数据流）优化
* 可使用的指令集包括SSE,SSE2,SSE3,SSSE3,SSE4.1,SSE4.2,AVX,AVX2,AVX512等
* 第二代Intel Core处理器以后的处理器一般可使用所有SSE指令集及AVX
* 第四代Intel Core处理器以后的处理器还可使用AVX2
* 启用SIMD向量化请取消对应宏的注释，同时在Visual Studio中开启增强指令集：
*     项目 -> 属性 -> C/C++-> 代码生成 -> 启用增强指令集
*/
#define EIGEN_VECTORIZE_SSE2
#define EIGEN_VECTORIZE_SSE3
#define EIGEN_VECTORIZE_SSSE3
#define EIGEN_VECTORIZE_SSE4_1
#define EIGEN_VECTORIZE_SSE4_2
#define EIGEN_VECTORIZE_AVX
//#define EIGEN_VECTORIZE_AVX2
/*
* Intel Math Kernel Library (MKL)
* MKL是Intel开发的核心数学库,使用该库可大幅提升矩阵运算速度
* 下载地址：https://software.intel.com/en-us/intel-mkl
* 启用MKL加速请取消对应宏的注释，同时在Visual Studio中启用MKL：
*     项目 -> 属性 -> Intel Performance Libraries -> Use Intel® MKL -> Parallel
*/
//#define EIGEN_USE_MKL_ALL	//使用Intel MKL
#include "Eigen/Eigen"

//-------------------------------Eigen Config End-------------------------------

#include <vector>			//STL序列容器：向量,内部使用动态数组实现
#include <limits>			//使用了numeric_limits函数确定ValueType类型的最大值
#include <initializer_list>	//初始化列表
#include <array>			//STL序列容器：在栈上分配定长数组,数组元素类型和数组长度由模板参数确定
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <list>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#ifdef _WIN32		//Windows平台
#	define pause() system("pause")
#else				//非Windows平台(Linux, Mac OS X)
#	define pause()
#endif

#ifndef _MSC_VER	//非Microsoft Visual C++编译器 (MinGW, GCC, Clang等)
/*
* 以下为了兼容非Microsoft Visual C++编译器,将fopen_s和fscanf_s宏定义为fopen和fscanf
* 使用G++或Clang编译器请将源文件保存为UTF-8无BOM格式,以正确显示中文
*/
#	define fopen_s(pStream, FileName, Mode) !(*pStream = fopen(FileName, Mode))
#	define fscanf_s(Stream, Format, ...) fscanf(Stream, Format, __VA_ARGS__)
#endif

using namespace std;
using namespace Eigen;
using namespace boost::property_tree;

#endif
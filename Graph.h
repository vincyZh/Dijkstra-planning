#ifndef _GRAPH_H_			//防止头文件被重复包含
#define _GRAPH_H_

#include "stdafx.h"

typedef pair<int, int> Line;

//有向图类型
template <typename ValueType>	//ValueType为有向图权值的类型,一般为int或double
class Graph
{
private:
	//使用Eigen矩阵库的稀疏矩阵类型存储关联矩阵
	SparseMatrix<ValueType> graph;

	ValueType getEdgeValue(int vs, int ve) const
	{
		return this->operator()(vs, ve);
	}

	/*
	 * 定义无穷大为ValueType类型的数值最大值
	 * int一般为2147483647, double一般为1.7×10e308
	 * static表示静态成员变量,每个Graph对象拥有相同的inf变量
	 * constexpr表示编译期确定常量
	 */
	static constexpr ValueType inf = numeric_limits<ValueType>::max();

public:
	/*
	 * 构造函数
	 * 模板参数Container为符合STL标准的关联容器,需要包含iterator类型和begin(),end()方法
	 * Container的模板参数：std::pair<Line,ValueType>
	 * 符合此标准的Container容器如下：
	 * std::set<std::pair<Line,ValueType>>
	 * std::vector<std::pair<Line,ValueType>>
	 * std::list<std::pair<Line,ValueType>>
	 * 等等
	 */
	template <typename Container>
	Graph(const Container &collection);

	//使用初始化列表构造
	Graph(const initializer_list < pair<Line, ValueType> > &initializer);

	//默认构造函数
	Graph(size_t numVertexes)
	{
		graph.resize(numVertexes, numVertexes);
	}

	//重载<<运算符,向有向图中添加边
	//返回当前对象的引用,用于对个<<运算符连用
	Graph &operator<<(const pair<Line,ValueType> &edge);

	//在有向图中查找vs->ve的权值,const表示常量成员函数,标明此函数不会修改成员变量
	ValueType operator()(int vs, int ve) const;

	//计算vs->ve的最短路径,详见此函数的实现部分
	ValueType shortestPath(int vs, int ve, vector<int> &edges) const;

	//计算vs->ve的结点数为m的最短路径,详见此函数的实现部分
	ValueType verticeConstrainedShortestPath(
		int vs, int ve, int m, vector<int> &edges
	);

	//去除所有边的权重
	void removeWeights();

	friend class GA;
};


/*
* 使用符合STL标准的容器构造Graph对象
* 典型构造方法：
* vector<pair<Line,double>> collection =
* {
*     { {1,2}, 3.5 },
*     { {2,3}, 4.2 }
* };
* Graph<double> graph(collection);
*/
template<typename ValueType>
template<typename Container>
inline Graph<ValueType>::Graph(const Container &collection)
{
	//确定关联矩阵的大小(图中顶点的个数)
	size_t size = 0;
	for (auto &item : collection)
	{
		const Line &edge = item.first;
		if (edge.first >= size)
			size = edge.first + 1;
		if (edge.second >= size)
			size = edge.second + 1;
	}
	graph.resize(size, size);
	//在关联矩阵中插入边
	for (auto &item : collection)
	{
		const Line &edge = item.first;
		graph.insert(edge.first, edge.second) = item.second;
	}
}

/*
* 使用构造函数初始化列表构造Graph对象
* 从而可以直接用{ }赋初值
* 典型构造方法：
* Graph<double> graph = 
* {
*     { {1,2}, 3.5},
*     { {2,3}, 4.2}
* };
*/
template <typename ValueType>
Graph<ValueType>::Graph(const initializer_list < pair<Line, ValueType> > &initializer)
{
	//确定关联矩阵的大小(图中顶点的个数)
	size_t size = 0;
	for (auto &item : initializer)
	{
		const Line &edge = item.first;
		if (edge.first >= size)
			size = edge.first + 1;
		if (edge.second >= size)
			size = edge.second + 1;
	}
	graph.resize(size, size);
	//在关联矩阵中插入边
	for (auto &item : initializer)
	{
		const Line &edge = item.first;
		graph.insert(edge.first, edge.second) = item.second;
	}
}


/*
 * 重载<<运算符,向有向图中添加边
 * 返回值：Graph对象的引用,实现多个<<运算符连用
 * 应用举例：
 * Graph<double> graph;
 * graph << pair<Line,double>({1,2},3.5) << pair<Line,double>({2,3},4.2);
 */
template <typename ValueType>
inline Graph<ValueType> &Graph<ValueType>::operator<<(const pair<Line, ValueType> &item)
{
	const Line &edge = item.first;
	assert(edge.first < graph.rows() && edge.second < graph.cols());
	graph.insert(edge.first, edge.second) = item.second;
	return *this;
}


//在有向图中查找vs->ve的权值
template <typename ValueType>
inline ValueType Graph<ValueType>::operator()(int vs, int ve) const
{
	//首尾结点相同,直接返回0
	if (vs == ve)
		return 0;
	//vs或ve不存在,直接返回∞
	if (vs >= graph.rows() || ve >= graph.cols())
		return inf;
	//在稀疏矩阵graph中查找vs->ve的权值
	ValueType value = graph.coeff(vs, ve);
	return value ? value : inf;
}


/*
 * @function name : shortestPath
 * @description : Dijkstra算法查找vs->ve的最短路径
 * @inparam : vs 起始结点
 * @inparam : ve 终止结点
 * @outparam : edges 最短路径
 * @return : 最短路径的长度
 */
template <typename ValueType>
inline ValueType Graph<ValueType>::shortestPath(int vs, int ve, vector<int> &edges) const
{
	//起始结点或终止结点不存在,则路径长度为∞
	if (vs >= graph.rows() || ve >= graph.cols())
		return inf;
	//定义本算法用到的数据结构
	size_t n = graph.size();
	bool *S = new bool[n];
	auto *dist = new ValueType[n];
	auto *path = new int[n];
	//初始化
	for (int v = 0; v < n; v++)
	{
		S[v] = false;
		dist[v] = getEdgeValue(vs, v);
		if (dist[v] < inf)
			path[v] = vs;
		else
			path[v] = -1;
	}
	dist[vs] = 0;
	S[vs] = true;
	//计算最短路径
	for (int i = 1; i < n; i++)
	{
		ValueType min = inf;
		int k = -1;
		for (int j = 0; j < n; j++)
		{
			if (!S[j] && dist[j] < min)
			{
				k = j;
				min = dist[j];
			}
		}
		if (k == -1)
			break;
		S[k] = true;
		for (int w = 0; w < n; w++)
		{
			ValueType distKW = getEdgeValue(k, w);
			if (dist[k] < inf &&
				distKW < inf
				&& !S[w]
				&& dist[k] + distKW < dist[w])
			{
				dist[w] = dist[k] + distKW;
				path[w] = k;
			}
		}
	}
	//转换为路径
	vector<int> rPath;	//反向路径
	for (int idx = ve; idx != vs; idx = path[idx])
	{
		if (idx == -1)	//路径上某结点的前驱结点是-1,无法达到vs,标明vs->ve不连通
		{
			delete[]path;
			delete[]S;
			delete[]dist;
			return inf;
		}
		rPath.push_back(idx);
	}
	rPath.push_back(vs);
	//反向路径->正向路径
	edges.resize(rPath.size());
	for (size_t i = 0; i < edges.size(); i++)
		edges[i] = rPath[edges.size() - i - 1];
	//释放内存
	ValueType minDist = dist[ve];
	delete[]path;
	delete[]S;
	delete[]dist;
	return minDist;
}



/*
 * @function name : verticeConstrainedShortestPath
 * @description : 求经过的结点数为k的vs->ve的最短路径
 * @inparam : vs 起始结点
 * @inparam : ve 终止结点
 * @inparam : k 经过的结点数(包括起点和终点)
 * @outparam : edges 最短路径
 * @return : 最短路径的长度
 */
template <typename ValueType>
inline ValueType Graph<ValueType>::verticeConstrainedShortestPath(
	int vs, int ve, int k, vector<int> &edges
)
{
	//最少包含起始和终点，k至少>=2
	if (k < 2)
		return inf;
	//定义本算法用到的数据结构
	size_t n = graph.rows();				//有向图的结点数
	ValueType *dist = new ValueType[n];		//vs->其他顶点的最短路径
	ValueType *pdist = new ValueType[n];	//上一轮循环的dist
	int **path = new int*[k - 1];			//存储每一轮循环的前驱结点
	for (size_t i = 0; i < k - 1; i++)
		path[i] = new int[n];
	//初始化
	for (int i = 0; i < n; i++)
	{
		dist[i] = getEdgeValue(vs, i);
		path[0][i] = vs;
		pdist[i] = dist[i];
	}
	//计算受结点数限制的最短路径
	for (int m = 1; m < k - 1; m++)
	{
		for (int j = 0; j < n; j++)
		{
			for (int i = 0; i < n; i++)
			{
				ValueType cost = getEdgeValue(i, j);
				if (pdist[i] < inf && cost < inf && dist[j] > pdist[i] + cost)
				{
					dist[j] = pdist[i] + cost;
					path[m][j] = i;
				}
			}
			if (dist[j] == pdist[j])
				path[m][j] = path[m - 1][j];
		}
		for (int u = 0; u < n; u++)
			pdist[u] = dist[u];
	}
	//输出路径
	edges.resize(k);
	int j = ve;
	for (int m = k - 2; m >= 0; m--)
	{
		edges[m] = path[m][j];
		j = path[m][j];
	}
	edges[k - 1] = ve;
	ValueType minDist = dist[ve];
	//释放内存
	delete[]dist;
	delete[]pdist;
	for (size_t i = 0; i < k - 1; i++)
		delete[]path[i];
	delete[]path;
	return minDist;
}

//去除所有边的权重,使之变成无权图
template<typename ValueType>
inline void Graph<ValueType>::removeWeights()
{
	for (int i = 0; i < graph.rows(); i++)
		for (int j = 0; j < graph.cols(); j++)
			if (graph.coeff(i, j) != 0)
				graph.coeffRef(i, j) = 1;
}

#endif // _GRAPH_H_
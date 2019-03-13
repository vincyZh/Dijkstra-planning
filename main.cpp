#include "stdafx.h"
#include "Graph.h"
#include "GA.h"

int START, END;

template <typename T>
struct ListOrder
{
	int NodeIdx;
	vector<int> path;
	T weight;
};



template <typename T>
size_t loadXML(const char *fileName,
	vector<pair<Line, T>> &edges,
	vector<NodeInfo<T>> &greens,
	int &start, int &end, int &requiredStep)
{
	edges.clear();
	greens.clear();
	ptree pt;
	read_xml(fileName, pt);
	//读取起始结点,终止结点,要求的步数
	for (auto &aGraph : pt.get_child("Graph"))
		if (aGraph.first == "<xmlattr>")
		{
			start = aGraph.second.get<int>("start");
			end = aGraph.second.get<int>("end");
			requiredStep = aGraph.second.get<int>("requiredStep");
		}
	//读取有向图所有边
	for (auto &vEdge : pt.get_child("Graph.Edges"))
		if (vEdge.first != "<xmlattr>" && vEdge.first != "<xmlcomment>")
			for (auto &aEdge : vEdge.second)
				if (aEdge.first == "<xmlattr>")
					edges.push_back(
				{
					{
						aEdge.second.get<int>("start"),
						aEdge.second.get<int>("end")
					},	aEdge.second.get<T>("weight")
				});
	//读取不经结点
	for (auto &vRedNode : pt.get_child("Graph.RedNodes"))
		if (vRedNode.first != "<xmlattr>" && vRedNode.first != "<xmlcomment>")
			for (auto &aRedNode : vRedNode.second)
				if (aRedNode.first == "<xmlattr>")
				{
					int node = aRedNode.second.get<int>("index");
					for (auto it = edges.begin(); it != edges.end();)
						if (it->first.first == node || it->first.second == node)
							it = edges.erase(it);
						else
							++it;
				}
	//读取不经线段
	for (auto &vRedEdge : pt.get_child("Graph.RedEdges"))
		if (vRedEdge.first != "<xmlattr>" && vRedEdge.first != "<xmlcomment>")
			for (auto &aRedEdge : vRedEdge.second)
				if (aRedEdge.first == "<xmlattr>")
				{
					int vs = aRedEdge.second.get<int>("start");
					int ve = aRedEdge.second.get<int>("end");
					T weight = aRedEdge.second.get<T>("weight");
					pair<Line, T> item({ vs,ve }, weight);
					edges.erase(find(edges.begin(), edges.end(), item));
				}
	//读取必经结点
	size_t NodeLineCnt = 0;
	for (auto &vGreenNode : pt.get_child("Graph.GreenNodes"))
		if (vGreenNode.first != "<xmlattr>" && vGreenNode.first != "<xmlcomment>")
			for (auto &aGreenNode : vGreenNode.second)
				if (aGreenNode.first == "<xmlattr>")
				{
					greens.push_back(
						NodeInfo<T>(aGreenNode.second.get<int>("index"))
					);
					NodeLineCnt++;
				}
	//读取必经线段
	for (auto &vGreenEdge : pt.get_child("Graph.GreenEdges"))
		if (vGreenEdge.first != "<xmlattr>" && vGreenEdge.first != "<xmlcomment>")
			for (auto &aGreenEdge : vGreenEdge.second)
				if (aGreenEdge.first == "<xmlattr>")
				{
					int vs = aGreenEdge.second.get<int>("start");
					int ve = aGreenEdge.second.get<int>("end");
					T weight = aGreenEdge.second.get<T>("weight");
					greens.push_back(NodeInfo<T>(vs, true, ve, weight));
					greens.push_back(NodeInfo<T>(ve, true, vs, weight));
					NodeLineCnt++;
				}
	return NodeLineCnt;
}

//确定需要的最少步数
template <typename T>
int minSteps(const Graph<T> &graph, vector<NodeInfo<T>> vecN, size_t NodeLineCnt)
{
	//创建无权图
	Graph<T> graphNoWeight = graph;
	graphNoWeight.removeWeights();

	for (auto &node : vecN)
	{
		node.weight = 1;
		node.isPassed = false;
	}

	//找经过所有必经结点的最短路径
	//存储必经结点的顺序和路径
	//NodeIdx是必经结点, weight是上一个结点到这个结点的距离,path是这一段的最短路径
	vector<ListOrder<T>> nodeOrder(NodeLineCnt + 1);
	nodeOrder[0].weight = inf;
	size_t index;
	for (size_t j = 0; j < vecN.size(); j++)
	{
		vector<int> path;
		T weight;
		weight = graphNoWeight.shortestPath(START, vecN[j].index, path);
		if (weight < nodeOrder[0].weight)
		{
			nodeOrder[0].weight = weight;
			nodeOrder[0].NodeIdx = vecN[j].index;
			nodeOrder[0].path = move(path);
			index = j;
		}
	}
	vecN[index].isPassed = true;
	if (vecN[index].isLine)
	{
		if (vecN[index].reIdx > vecN[index].index)
			vecN[index + 1].isPassed = true;
		else
			vecN[index - 1].isPassed = true;
		nodeOrder[0].NodeIdx = vecN[index].reIdx;
		nodeOrder[0].path.push_back(vecN[index].reIdx);
		nodeOrder[0].weight += vecN[index].weight;
	}


	for (size_t i = 1; i < NodeLineCnt; i++)
	{
		nodeOrder[i].weight = inf;
		for (size_t j = 0; j < vecN.size(); j++)
		{
			if (vecN[j].isPassed)
				continue;
			vector<int> path;
			T weight;
			weight = graphNoWeight.shortestPath(nodeOrder[i - 1].NodeIdx, vecN[j].index, path);
			if (weight < nodeOrder[i].weight)
			{
				nodeOrder[i].weight = weight;
				nodeOrder[i].NodeIdx = vecN[j].index;
				nodeOrder[i].path = move(path);
				index = j;
			}
		}
		vecN[index].isPassed = true;
		if (vecN[index].isLine)
		{
			if (vecN[index].reIdx > vecN[index].index)
				vecN[index + 1].isPassed = true;
			else
				vecN[index - 1].isPassed = true;
			nodeOrder[i].NodeIdx = vecN[index].reIdx;
			nodeOrder[i].path.push_back(vecN[index].reIdx);
			nodeOrder[i].weight += vecN[index].weight;
		}
	}
	nodeOrder[NodeLineCnt].NodeIdx = END;
	nodeOrder[NodeLineCnt].weight =
		graphNoWeight.shortestPath(nodeOrder[NodeLineCnt - 1].NodeIdx, END, nodeOrder[NodeLineCnt].path);
	
	int weightSum = 0;
	for (auto it_pathseg = nodeOrder.begin(); it_pathseg != nodeOrder.end(); ++it_pathseg)
		weightSum += static_cast<int>(it_pathseg->weight);
	return weightSum + 1;
}


int main()
{
	srand(static_cast<unsigned int>(time(nullptr)));
	//读取原始数据
	vector<pair<Line, double>> data;
	vector<NodeInfo<double>> vecN;
	int requiredStep;
	size_t NodeLineCnt;
	try
	{
		NodeLineCnt = loadXML("Graph.xml", data, vecN, START, END, requiredStep);
	}
	catch (...)
	{
		printf("Graph.xml解析失败\n");
		pause();
		return 0;
	}

	//初始化有向图
	Graph<double> graph(data);


	//存储必经结点的顺序和路径
	//NodeIdx是必经结点, weight是上一个结点到这个结点的距离,path是这一段的最短路径
	vector<ListOrder<double>> nodeOrder(NodeLineCnt + 1);
	nodeOrder[0].weight = inf;
	size_t index;
	for (size_t j = 0; j < vecN.size(); j++)
	{
		vector<int> path;
		double weight;
		weight = graph.shortestPath(START, vecN[j].index, path);
		if (weight < nodeOrder[0].weight)
		{
			nodeOrder[0].weight = weight;
			nodeOrder[0].NodeIdx = vecN[j].index;
			nodeOrder[0].path = move(path);
			index = j;
		}
	}
	vecN[index].isPassed = true;
	if (vecN[index].isLine)
	{
		if (vecN[index].reIdx > vecN[index].index)
			vecN[index + 1].isPassed = true;
		else
			vecN[index - 1].isPassed = true;
		nodeOrder[0].NodeIdx = vecN[index].reIdx;
		nodeOrder[0].path.push_back(vecN[index].reIdx);
		nodeOrder[0].weight += vecN[index].weight;
	}


	for (size_t i = 1; i < NodeLineCnt; i++)
	{
		nodeOrder[i].weight = inf;
		for (size_t j = 0; j < vecN.size(); j++)
		{
			if (vecN[j].isPassed)
				continue;
			vector<int> path;
			double weight;
			weight = graph.shortestPath(nodeOrder[i - 1].NodeIdx, vecN[j].index, path);
			if (weight < nodeOrder[i].weight)
			{
				nodeOrder[i].weight = weight;
				nodeOrder[i].NodeIdx = vecN[j].index;
				nodeOrder[i].path = move(path);
				index = j;
			}
		}
		vecN[index].isPassed = true;
		if (vecN[index].isLine)
		{
			if (vecN[index].reIdx > vecN[index].index)
				vecN[index + 1].isPassed = true;
			else
				vecN[index - 1].isPassed = true;
			nodeOrder[i].NodeIdx = vecN[index].reIdx;
			nodeOrder[i].path.push_back(vecN[index].reIdx);
			nodeOrder[i].weight += vecN[index].weight;
		}
	}
	nodeOrder[NodeLineCnt].NodeIdx = END;
	nodeOrder[NodeLineCnt].weight =
		graph.shortestPath(nodeOrder[NodeLineCnt - 1].NodeIdx, END, nodeOrder[NodeLineCnt].path);


	//输出路径
	printf("总权值最小的路径（不考虑限定步数）\t");
	double weightSum = 0;
	int numNodesPassed = 0;
	for (auto it_pathseg = nodeOrder.begin(); it_pathseg != nodeOrder.end(); ++it_pathseg)
	{
		if (it_pathseg == nodeOrder.begin())
		{
			cout << it_pathseg->path[0] << " ";
			numNodesPassed++;
		}
		for (size_t i = 1; i < it_pathseg->path.size(); i++)
		{
			cout << it_pathseg->path[i] << " ";
			numNodesPassed++;
		}
		weightSum += it_pathseg->weight;
	}
	printf("\n总权值为 %lf\t经过的总结点数 %d\n", weightSum, numNodesPassed);
	printf("\n不考虑权值最小,经过的总结点数最少为 %d\n\n", minSteps(graph, vecN, NodeLineCnt));

	//遗传算法计算考虑最优路径
	GA ga(graph, vecN, 0, 17);
	vector<int> bestPath = ga.PrintBest();

	printf("\n最优路径 ");
	for (auto &p : bestPath)
		cout << p << ' ';
	pause();
	return 0;
}
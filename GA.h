#pragma once
#include "stdafx.h"
#include "Graph.h"

constexpr double inf = numeric_limits<double>::max();

template <typename T>
struct NodeInfo
{
	int index;		//当前结点的index
	bool isPassed;	//当前结点是否已经经过
	bool isLine;	//是否为线段
	int reIdx;		//如果为线段,则存储线段的另一个端点的index
	T weight;		//如果为线段,则存储线段的权重
	NodeInfo(int _index, bool _isLine = false, int _reIdx = -1, T _weight = inf)
		:index(_index), isPassed(false),
		isLine(_isLine), reIdx(_reIdx), weight(_weight) {}
};

class GA
{
private:
	size_t PATH_LENGTH;
	size_t GA_POPSIZE;
	size_t GA_MAXITER;
	const double GA_ELITRATE = 0.1;	//交叉互换过程中保留的最适种群比例
	int START, END;
	Graph<double> graph;
	vector<NodeInfo<double>> vecN;
	struct GA_struct
	{
		vector<int> path;
		double fitness;
	};

	void CalFitness(GA_struct &citizen)
	{
		citizen.fitness = 0;
		for (size_t i = 1; i < PATH_LENGTH; i++)
			citizen.fitness += graph.graph.coeff(citizen.path[i - 1], citizen.path[i]);
		size_t numN = 0;
		vector<bool> isNPassed(vecN.size());
		for (size_t i = 0; i < PATH_LENGTH; i++)
		{
			for (size_t j = 0; j < vecN.size(); j++)
				if (citizen.path[i] == vecN[j].index && !isNPassed[j])
				{
					if (!vecN[j].isLine)
					{
						numN++;
						isNPassed[j] = true;
					}
					else if (i < PATH_LENGTH - 1 && citizen.path[i + 1] == vecN[j].reIdx)
					{
						numN++;
						isNPassed[j] = true;
						//相关联的线段端点也被认为已经过
						if (vecN[j].reIdx > vecN[j].index)
							isNPassed[j + 1] = true;
						else
							isNPassed[j - 1] = true;
					}
				}
		}
		citizen.fitness = citizen.fitness * (1 << (4 - numN));
	}

	void InitPopulation(vector<GA_struct> &population)
	{
		population.resize(GA_POPSIZE);
#pragma omp parallel for
		for (int i = 0; i < GA_POPSIZE; i++)
		{
			do
			{
				population[i].path.resize(PATH_LENGTH);
				int n = graph.graph.rows();
				int col;
				//生成起点->第1个结点
				do
				{
					col = rand() % n;
				} while (graph.graph.coeff(START, col) == 0 || col == START || col == END);
				population[i].path[0] = START;
				population[i].path[1] = col;
				//生成中间的结点
				for (size_t j = 2; j < PATH_LENGTH; j++)
				{
					do
					{
						col = rand() % n;
					} while (graph.graph.coeff(population[i].path[j - 1], col) == 0 || col == START);
					population[i].path[j] = col;
				}
			} while (population[i].path[PATH_LENGTH - 1] != END);
			CalFitness(population[i]);
		}

	}

	bool isNotConnected(int i1, int i2)
	{
		return graph.graph.coeff(i1, i2) == 0;
	}

	void Mate(vector<GA_struct> &population)
	{
		size_t esize = GA_POPSIZE * GA_ELITRATE;
		vector<GA_struct> buffer(population.size());
		for (auto &item : buffer)
			item.path.resize(PATH_LENGTH);
		for (size_t i = 0; i < esize; i++)
			buffer[i] = population[i];
#pragma omp parallel for
		for (int i = esize; i < GA_POPSIZE; i++)
		{
			size_t j, start, end;
			do 
			{
				j = rand() % (GA_POPSIZE / 2);
				start = rand() % PATH_LENGTH;
				do
				{
					end = rand() % PATH_LENGTH;
				} while (end < start);
			} while (
				(start == 0 && end != PATH_LENGTH - 1 && isNotConnected(population[i].path[end + 1], population[j].path[end])) ||
				(start != 0 && end == PATH_LENGTH - 1 && isNotConnected(population[i].path[start - 1], population[j].path[start])) ||
				(start != 0 && end != PATH_LENGTH - 1 && (isNotConnected(population[i].path[start - 1], population[j].path[start]) || isNotConnected(population[i].path[end + 1], population[j].path[end])))
			);
			for (size_t k = 0; k < start; k++)
				buffer[i].path[k] = population[i].path[k];	//0 ~ start-1
			for (size_t k = start; k <= end; k++)
				buffer[i].path[k] = population[j].path[k];	//start ~ end
			for (size_t k = end + 1; k < PATH_LENGTH; k++)
				buffer[i].path[k] = population[i].path[k];	//end ~ PATH_LENGTH-1
		}
		population = move(buffer);
	}

	void Evolute(vector<GA_struct> &population, double sum_fitness)
	{
		size_t pos = 0;
		for (auto &citizen : population)
		{
			size_t cp = (1 - citizen.fitness / sum_fitness) * GA_POPSIZE;
			if (pos >= GA_POPSIZE)
				break;
			if (pos + cp < GA_POPSIZE)
				fill_n(population.begin() + pos, cp, citizen);
			else
				fill(population.begin() + pos, population.end(), citizen);
			pos += cp;
		}
	}

public:
	vector<int> PrintBest()
	{
		vector<GA_struct> population;
		InitPopulation(population);
		list<double> preFitness;
		for (size_t i = 0; i < GA_MAXITER; i++)
		{
			Mate(population);
			//计算适应度
			double sum_fitness = 0;
			for (auto &citizen : population)
			{
				CalFitness(citizen);
				sum_fitness += citizen.fitness;
			}
			//按适应度从小到大排序
			sort(population.begin(), population.end(), [](GA_struct &x, GA_struct &y)
			{
				return x.fitness < y.fitness;
			});
			printf("第%zd次循环的最优值\t%lf\n", i, population[0].fitness);
			//判断是否达到迭代终止条件
			if (preFitness.size() >= 20)
			{
				bool isEqual = true;
				double fitness = *preFitness.begin();
				for (auto &item : preFitness)
					if (item != fitness)
						isEqual = false;
				if (isEqual)
					break;
				preFitness.pop_front();
			}

			preFitness.push_back(population[0].fitness);	
		}
		return population[0].path;
	}

	GA(Graph<double> &_graph, vector<NodeInfo<double>> &_vecN, int start = 0, int end = 17, size_t path_length = 12, size_t popsize = 655, size_t maxiter = 65536)
		: graph(_graph), START(start), END(end), vecN(_vecN),
		PATH_LENGTH(path_length), GA_POPSIZE(popsize), GA_MAXITER(maxiter) {}
	//START: 起始结点, END: 终止结点, vecN: 必经结点和线段, PATH_LENGTH: 要求的步数（经过的总结点数）
	//GA_POPSIZE: 种群大小, GA_MAXITER: 最大迭代次数
};
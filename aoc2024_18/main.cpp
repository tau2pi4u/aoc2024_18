#include <array>
#include <map>
#include "utils.hpp"

#define TESTING 0
#if TESTING
#define INFILE "testInput.txt"
#define P1_TIME 12
#define GX 7
#define GY 7
#else
#define INFILE "input.txt"
#define P1_TIME 1024
#define GX 71
#define GY 71
#endif

#define VISUALISE 0

struct Node
{
	int cost = INT32_MAX;
	int timeBlocked = INT32_MAX;
	int x;
	int y;
	Node* forward = nullptr;
	std::array<Node*, AsInt(Direction::Count)> neighbours;
};

struct Graph
{
	Graph(std::vector<std::string> const& lines, size_t gX, size_t gY) :
		gX(gX),
		gY(gY),
		nodes(gX, gX * gY)
	{
		nodes.resize(gX * gY);

		// should this be one?
		int time = 0;
		for (auto& line : lines)
		{
			int x, y;
			SCANF(line.c_str(), "%d,%d", &x, &y);

			nodes[y][x].timeBlocked = time;
			++time;
		}

		for (int y = 0; y < gY; ++y)
		{
			for (int x = 0; x < gX; ++x)
			{
				for (auto [dirInt, dir] : DirectionIterator())
				{
					nodes[y][x].x = x;
					nodes[y][x].y = y;
					if (auto neighbour = nodes.GetPointerIfInBounds(y, x, dir, 1))
					{
						nodes[y][x].neighbours[dirInt] = neighbour;
					}
				}
			}
		}
	}

	void Reset()
	{
		for (auto& node : nodes)
		{
			node.cost = INT32_MAX;
			node.forward = nullptr;
		}
	}

	size_t gX;
	size_t gY;
	TwoDVector<Node> nodes;
};

int Dijkstras1(Graph& g, int time)
{
	std::map<int, std::vector<Node*>> candidates;
	Node* endState = &g.nodes[0][0];
	Node* startState = &g.nodes[g.gY - 1][g.gX - 1];

	startState->cost = 0;
	candidates[0].push_back(startState);

	int i = 0;
	while (++i && !candidates.empty())
	{
		auto [_cost, _nodes] = *candidates.begin();
		candidates.erase(candidates.begin());

		for (auto& node : _nodes)
		{
			if (node == endState)
			{
				return endState->cost;
			}

			if (node->timeBlocked < time) continue;

			for (auto& neighbour : node->neighbours)
			{
				if (!neighbour) continue;
				int newCost = node->cost + 1;
				if (newCost >= neighbour->cost) continue;

				if (neighbour->cost != INT32_MAX)
				{
					EasyErase(candidates[neighbour->cost], neighbour);
					if (candidates[neighbour->cost].empty())
					{
						candidates.erase(neighbour->cost);
					}
				}

				neighbour->forward = node;
				neighbour->cost = newCost;
				candidates[neighbour->cost].push_back(neighbour);
			}
		}
	}
	return -1;
}

int Part2(Graph& g, int maxTime)
{
	int time = maxTime / 2;
	int step = time / 2;

	while (step)
	{
		// can find a way out, add time
		g.Reset();
		if (Dijkstras1(g, time) > 0)
		{
			time += step;
		}
		else
		{
			time -= step;
		}

		step /= 2;
	}

	for (; time < maxTime; ++time)
	{
		g.Reset();
		if (Dijkstras1(g, time) == -1)
		{
			break;
		}
	}

	for (; time > 0; --time)
	{
		g.Reset();
		if (Dijkstras1(g, time) > 0)
		{
			break;
		}
	}

	return time;
}

#if VISUALISE
void DumpForward(Graph& g, int time, int iter, const char * prefix)
{
	char outbuf[256] = { 0 };
	FILE* fout;

	sprintf_s(outbuf, 256, "vis/%s/%d.txt", prefix, iter);
	fopen_s(&fout, outbuf, "w");
	if (!fout) return;

	TwoDVector<bool> path(g.nodes.XDim());
	path.resize(g.nodes._vec.size());

	auto node = &g.nodes[0][0];
	while (node)
	{
		path.set(node->y, node->x, true);
		node = node->forward;
	}

	for (int y = 0; y < g.gY; ++y)
	{
		for (int x = 0; x < g.gX; ++x)
		{
			bool isBlocked = g.nodes[y][x].timeBlocked < time;
			bool isPath = path.get(y, x);

			if (isBlocked && isPath) Unreachable();

			if (isBlocked) fprintf_s(fout, "#");
			else if (isPath) fprintf_s(fout, "O");
			else fprintf_s(fout, ".");

		}
		fprintf_s(fout, "\n");
	}
	fclose(fout);
}

int DrawPart2Slow(Graph& g, int maxTime)
{
	int time = 0;

	for (; time < maxTime; ++time)
	{
		g.Reset();
		auto res = Dijkstras1(g, time);
		DumpForward(g, time, time, "slow");
		if(res == -1)
		{
			break;
		}
	}

	for (; time > 0; --time)
	{
		g.Reset();
		if (Dijkstras1(g, time) > 0)
		{
			break;
		}
	}

	return time;
}

int DrawPart2Fast(Graph& g, int maxTime)
{
	int time = maxTime / 2;
	int step = time / 2;
	int iter = 0;

	while (step)
	{
		// can find a way out, add time
		g.Reset();
		if (Dijkstras1(g, time) > 0)
		{
			DumpForward(g, time, iter, "fast");
			time += step;
		}
		else
		{
			DumpForward(g, time, iter, "fast");
			time -= step;
		}

		step /= 2;
		++iter;
	}

	for (; time < maxTime; ++time, ++iter)
	{
		g.Reset();
		if (Dijkstras1(g, time) == -1)
		{
			DumpForward(g, time, iter, "fast");
			break;
		}
		DumpForward(g, time, iter, "fast");
	}

	for (; time > 0; --time, ++iter)
	{
		g.Reset();
		if (Dijkstras1(g, time) > 0)
		{
			DumpForward(g, time, iter, "fast");
			break;
		}
		DumpForward(g, time, iter, "fast");
	}

	++time;
	g.Reset();
	Dijkstras1(g, time);
	DumpForward(g, time, ++iter, "fast");

	return time;
}

#endif

#include <chrono>

int main()
{
	auto start = std::chrono::high_resolution_clock::now();
	auto inputLines = GetInputAsString(INFILE);
	Graph g(inputLines, GX, GY);
	auto p1 = Dijkstras1(g, P1_TIME);
	auto p2 = Part2(g, static_cast<int>(inputLines.size()));
	auto end = std::chrono::high_resolution_clock::now();

	printf("p1: %d\n", p1);
	printf("%s\n", inputLines[p2].c_str());
	printf("%lld us\n", (end - start).count() / 1000);

#if VISUALISE

	DrawPart2Slow(g, static_cast<int>(inputLines.size()));
	DrawPart2Fast(g, static_cast<int>(inputLines.size()));

#endif
}
#include <bits/stdc++.h>
using namespace std;

const int Gw = 3;
const int Gl = 3;
const int Gh = 3;

struct Application
{
    int id;
    vector<int> tasks;
    vector<vector<int>> edges;
    vector<double> commVolume;
    int NOL = -1;
    int MD = -1;
    double avgCommValue;
    int xmin = INT_MAX, xmax = INT_MIN, ymin = INT_MAX, ymax = INT_MIN;
    int startX, startY, startZ;
    void computeAvg()
    {
        if (!commVolume.empty())
        {
            avgCommValue = accumulate(commVolume.begin(), commVolume.end(), 0.0) / commVolume.size();
        }
        else
        {
            avgCommValue = 0.0; // Handle empty vector case
        }
    }
};

struct Core
{
    int isFree = -1;
    int x, y, z;
};

Core NoC[Gw][Gl][Gh];
int Pc[Gw][Gl][Gh];
std::vector<int> freecores((int)Gh, (int)(Gw *Gl));
using AppIntPair = pair<vector<Application>, vector<int>>;

double ERT(int a, double v, int nol, int md)
{
    // double c0 = 5.0, c1 = 1.0, c2 = 3.0, c3 = 8.0, c4 = 2.0;
    double c0 = 1.0, c1 = 1.0, c2 = 1.0, c3 = 1.0, c4 = 1.0;
    double ans = (double)c0 + ((double)c1 * a) + ((double)c2 * v) + ((double)c3 * nol) + ((double)c4 * md);
    return ans;
}

int isnotLeaf(vector<Application> &apps)
{
    int t = 0;
    for (auto &app : apps)
    {
        if (app.NOL == -1 || app.MD == -1)
            t = 1;
    }
    return t;
}

double lowerbound_ert(Application app)
{
    int max_task_per_layer = Gw * Gl;
    int nol = (int)(ceil((double)(app.tasks.size() / max_task_per_layer))) + 1;
    if (nol > Gh)
    {
        std::cerr << "Application with " << app.tasks.size() << " tasks requires "
                  << nol << " layers, which exceeds the available height " << Gh << ". Skipping.\n";
        return -1.0;
    }
    int md = 0;
    double ert = ERT(app.tasks.size(), app.avgCommValue, nol, md);
    return ert;
    // std::cout << "Application (" << app.tasks.size()  << " tasks, avg_comm = "
    //           << app.avgCommValue << "): NOL_min = " << nol
    //           << ", MD_min = " << md << ", ERT = " << ert << "\n";
}

double computeSigmaStar(const std::vector<Application> &apps)
{
    double sigmaStar = 0;
    for (const auto &app : apps)
    {
        double ert = lowerbound_ert(app);
        if (ert == -1.0)
            continue;
        sigmaStar += ert;
    }
    return sigmaStar;
}

AppIntPair findCoreRegionShape(vector<Application> &apps)
{
    queue<AppIntPair> WQ;
    AppIntPair NBN, BN, LBN;
    NBN.first = apps;
    NBN.second = freecores;
    double sigmaStar = computeSigmaStar(NBN.first) + 6;
    double leafNode_sigmastar = INT_MAX;
    int x = 0;
    WQ.push(NBN);
    int counter = 0;
    int marker = 1;
    while (WQ.size() != 0)
    {
        // if (counter == 6)
        //     break;
        // counter++;
        AppIntPair Nq = WQ.front();
        WQ.pop();
        int x = 0;
        for (int i = 0; i < Nq.first.size(); i++)
        {
            if (Nq.first[i].NOL == -1)
            {
                x = i;
                break;
            }
        }
        if (isnotLeaf(Nq.first))
        {
            vector<Application> buf = Nq.first;
            vector<int> buf2 = Nq.second;
            int nolmin = ((buf[x].tasks.size()) / (Gw * Gl)) + 1;
            int nolmax = min(Gh, (int)(buf[x].tasks.size()));
            int mdmin = 0;
            for (int i = 0; i < buf2.size(); i++)
            {
                if (buf2[i] != 0)
                {
                    mdmin = i;
                    break;
                }
            }
            int mdmax = (int)Gh - nolmin;
            for (int i = mdmin; i <= mdmax; i++)
            {
                for (int j = nolmin; j <= nolmin; j++)
                {
                    buf[x].MD = i;
                    buf[x].NOL = j;
                    int feasible = 1;
                    if (i + j > (int)Gh)
                    {
                        feasible = 0;
                    }
                    int avgcores = buf[x].tasks.size() / j;
                    for (int k = i; k < i + j && k <= Gh; k++)
                    {
                        if (avgcores > buf2[k])
                        {
                            feasible = 0;
                            break;
                        }
                    }
                    int sigmaMin = 0;
                    for (int i = 0; i < x; i++)
                    {
                        sigmaMin += ERT(buf[i].tasks.size(), buf[i].avgCommValue, buf[i].NOL, buf[i].MD);
                    }
                    sigmaMin += ERT(buf[x].tasks.size(), buf[i].avgCommValue, i, j);
                    for (int i = x + 1; i < Nq.first.size(); i++)
                    {
                        double ert = lowerbound_ert(Nq.first[i]);
                        sigmaMin += ert;
                    }
                    if (sigmaMin >= sigmaStar)
                    {
                        feasible = 0;
                    }
                    if (feasible)
                    {
                        for (int k = i; k < i + j && k <= Gh; k++)
                        {
                            buf2[k] -= avgcores;
                        }
                        pair<vector<Application>, vector<int>> temp;
                        temp.first = buf;
                        temp.second = buf2;
                        WQ.push(temp);
                        for (int k = i; k < i + j && k <= Gh; k++)
                        {
                            buf2[k] += avgcores;
                        }
                    }
                }
            }
        }
        else
        {
            double sigma = 0;
            for (auto app : Nq.first)
            {
                sigma += ERT(app.tasks.size(), app.avgCommValue, app.NOL, app.MD);
            }
            if (sigma < sigmaStar)
            {
                sigmaStar = sigma;
                BN = Nq;
                marker = 0;
            }
            if (sigma <= leafNode_sigmastar)
            {
                leafNode_sigmastar = sigma;
                LBN = Nq;
            }
        }
        if (marker)
        {
            BN = LBN;
        }
    }

    return BN;
}

int main()
{
    vector<Application> apps = {
        {1, {1, 2, 3, 4, 5}, {{1, 2, 4}, {1, 3, 4}, {2, 4, 4}, {4, 5, 4}}, {3, 4, 5, 3}},
        {2, {3, 4, 2, 3, 1}, {{2, 4, 3}, {3, 2, 4}, {5, 2, 3}}, {6, 2, 2}},
        {3, {2, 3, 9, 1}, {{3, 4, 1}, {1, 4, 8}}, {4, 9, 2}}};
    AppIntPair ans = findCoreRegionShape(apps);
    for (auto x : ans.second)
        cout << x << " ";
    for (auto app : ans.first)
    {
        cout << app.id << " " << app.MD << " " << app.NOL << endl;
    }
}

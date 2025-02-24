#include <bits/stdc++.h>
using namespace std;

const int Gw = 3;
const int Gl = 3;
const int Gh = 3;
int glbmark = 1;

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

bool findCoreRegionLocation(Application &app, int cornerIndex)
{
    int width = ceil(sqrt(app.tasks.size() / (double)app.NOL));
    int length = ceil(sqrt(app.tasks.size() / (double)app.NOL));

    int startX, startY, startZ, dirX, dirY;
    if (cornerIndex % 4 == 0)
    {
        startX = 0;
        startY = 0;
        startZ = app.MD;
        dirX = 1;
        dirY = 1;
    }
    else if (cornerIndex % 4 == 1)
    {
        startX = Gw - 1;
        startY = 0;
        startZ = app.MD;
        dirX = -1;
        dirY = 1;
    }
    else if (cornerIndex % 4 == 2)
    {
        startX = Gw - 1;
        startY = Gl - 1;
        startZ = app.MD;
        dirX = -1;
        dirY = -1;
    }
    else
    {
        startX = 0;
        startY = Gl - 1;
        startZ = app.MD;
        dirX = 1;
        dirY = -1;
    }

    for (int y = startY; (dirY > 0 ? y < Gl : y >= 0); y += dirY)
    {
        for (int x = startX; (dirX > 0 ? x < Gw : x >= 0); x += dirX)
        {
            int available = 0;
            int zmark = startZ;
            for (int k = startZ; k <= Gh - app.NOL; k++)
            {
                available = 0;
                for (int z = k; z < k + app.NOL; z++)
                {
                    for (int dy = 0; dy < length; ++dy)
                    {
                        for (int dx = 0; dx < width; ++dx)
                        {
                            int nx = x + dx * dirX;
                            int ny = y + dy * dirY;
                            if (nx >= 0 && nx < Gw && ny >= 0 && ny < Gl && NoC[nx][ny][z].isFree == -1)
                            {
                                available++;
                            }
                        }
                    }
                }
                if (available >= app.tasks.size() || glbmark == 1)
                {
                    zmark = k;
                    break;
                }
            }
            if (available < app.tasks.size())
                continue;

            int assigned = 0;
            for (int z = zmark; z < zmark + app.NOL && assigned < app.tasks.size(); z++)
            {
                for (int dy = 0; dy < length && assigned < app.tasks.size(); dy++)
                {
                    for (int dx = 0; dx < width && assigned < app.tasks.size(); dx++)
                    {
                        int nx = x + dx * dirX;
                        int ny = y + dy * dirY;
                        if (nx >= 0 && nx < Gw && ny >= 0 && ny < Gl && NoC[nx][ny][z].isFree == -1)
                        {
                            NoC[nx][ny][z].isFree = app.id;
                            app.xmax = max(app.xmax, nx);
                            app.xmin = min(app.xmin, nx);
                            app.ymax = max(app.ymax, ny);
                            app.ymin = min(app.ymin, ny);
                            assigned++;
                        }
                    }
                }
            }
            app.startX = startX;
            app.startY = startY;
            app.startZ = startZ;
            return true;
        }
    }
    return false;
}

int main()
{
    vector<Application> apps = {
        {1, {1, 2, 3, 4, 5}, {{1, 2, 4}, {1, 3, 4}, {2, 4, 4}, {4, 5, 4}}, {3, 4, 5, 3}},
        {2, {3, 4, 2, 3, 1}, {{2, 4, 3}, {3, 2, 4}, {5, 2, 3}}, {6, 2, 2}},
        {3, {2, 3, 9, 1}, {{3, 4, 1}, {1, 4, 8}}, {4, 9, 2}}};
    apps[0].NOL = 1, apps[0].MD = 0;
    apps[1].NOL = 1, apps[1].MD = 1;
    apps[2].NOL = 1, apps[2].MD = 0;
    int cornerIndex = 0;
    for (auto &app : apps)
    {
        // if (cornerIndex == 2)
        //     cornerIndex += 1;
        for (int x = 1; x <= 4; x++)
        {
            bool possible = findCoreRegionLocation(app, cornerIndex % 4);
            if (!possible)
            {
                cornerIndex++;
            }
            else
            {
                break;
            }
        }
        cornerIndex++;
    }
    for (auto app : apps)
    {
        cout << app.id << " " << app.MD << " " << app.NOL << endl;
        cout << app.xmax << " " << app.xmin << endl;
        cout << app.ymax << " " << app.ymin << endl;
    }
    cout << endl;
    cout << "Final state of the NoC:" << endl;
    for (int z = 0; z < Gh; ++z)
    {
        cout << "Layer " << z << ":" << endl;
        for (int y = 0; y < Gl; ++y)
        {
            for (int x = 0; x < Gw; ++x)
            {
                cout << NoC[x][y][z].isFree << " ";
                // cout << (NoC[x][y][z].isFree == -1 ? '.' : NoC[x][y][z].isFree);
            }
            cout << endl;
        }
        cout << endl;
    }
}
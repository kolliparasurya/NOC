#include <bits/stdc++.h>
using namespace std;

const int Gw = 3;
const int Gl = 3;
const int Gh = 3;
int glbmark = 0;

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

vector<Core> lineFreeCoreCount(Core current, char direction)
{
    vector<Core> freeCores;
    int dx = 0, dy = 0;
    if (direction == 'X')
        dx = 1;
    else if (direction == 'Y')
        dy = 1;
    if (current.isFree == -1)
        freeCores.push_back(current);
    for (int i = 1; i < max(Gw, Gl); ++i)
    {
        int nx = current.x + i * dx;
        int ny = current.y + i * dy;
        if (nx >= 0 && nx < Gw && ny >= 0 && ny < Gl && NoC[nx][ny][current.z].isFree == -1)
            freeCores.push_back(NoC[nx][ny][current.z]);
        else
            break;
    }
    for (int i = 1; i < max(Gw, Gl); ++i)
    {
        int nx = current.x - i * dx;
        int ny = current.y - i * dy;
        if (nx >= 0 && nx < Gw && ny >= 0 && ny < Gl && NoC[nx][ny][current.z].isFree == -1)
            freeCores.push_back(NoC[nx][ny][current.z]);
        else
            break;
    }
    return freeCores;
}

int calculateCenterFreeCores()
{
    int centerFreeCores = 0;
    for (int z = 0; z < Gh; ++z)
    {
        Core centerCore = NoC[Gw / 2][Gl / 2][z];
        vector<Core> xFreeCores = lineFreeCoreCount(centerCore, 'X');
        for (auto &core : xFreeCores)
        {
            vector<Core> yFreeCores = lineFreeCoreCount(core, 'Y');
            centerFreeCores += yFreeCores.size();
        }
    }
    cout << centerFreeCores << endl;
    return centerFreeCores;
}

int main()
{
    NoC[0][0][0].isFree = 1;
    NoC[1][0][0].isFree = 1;
    NoC[2][0][0].isFree = 1;
    NoC[0][1][0].isFree = 1;
    NoC[1][1][0].isFree = 1;

    NoC[2][0][1].isFree = 2;
    NoC[1][0][1].isFree = 2;
    NoC[0][0][1].isFree = 2;
    NoC[2][1][1].isFree = 2;
    NoC[1][1][1].isFree = 2;

    // NoC[2][2][2].isFree = 3;
    // NoC[2][1][2].isFree = 3;
    // NoC[1][2][2].isFree = 3;
    // NoC[1][1][2].isFree = 3;

    for (int i = 0; i < 3; i++)
    {
        cout << "layer:" << i << endl;
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                cout << NoC[k][j][i].isFree << " ";
                NoC[k][j][i].x = k;
                NoC[k][j][i].y = j;
                NoC[k][j][i].z = i;
            }
            cout << endl;
        }
        cout << endl;
    }
    int ans = calculateCenterFreeCores();
    cout << endl;
    cout << ans << endl;
}
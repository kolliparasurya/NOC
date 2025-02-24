#include <bits/stdc++.h>
using namespace std;

const int Gw = 5;
const int Gl = 5;
const int Gh = 2;
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
    return centerFreeCores;
}

// pair<pair<int, int>, pair<int, int>>
pair<pair<int, int>, pair<int, int>> findVirtualMigrationPaths(Application &app, int dx, int dy)
{
    pair<int, int> pathXY, pathYX;
    int xpos, ypos;
    if (dx == -1)
    {
        xpos = app.xmin;
    }
    else if (dx == 1)
    {
        xpos = app.xmax;
    }
    if (dy == -1)
    {
        ypos = app.ymin;
    }
    else if (dy == 1)
    {
        ypos = app.ymax;
    }
    int mark = 0;
    int tempxXY = 0, tempyXY = 0;
    while (1)
    {
        xpos += dx;
        for (int z = app.startZ; z < app.startZ + app.NOL; z++)
        {
            int inner = 0;
            for (int y = app.ymin; y <= app.ymax; y++)
            {
                if (NoC[xpos][y][z].isFree != -1 || y < 0 || y >= Gl || xpos >= Gw || xpos <= -1)
                {
                    mark = 1;
                    inner = 1;
                    break;
                }
            }
            if (inner == 1)
                break;
        }
        if (mark == 1 || xpos >= Gw || xpos <= -1)
            break;
        tempxXY++;
    }
    mark = 0;
    while (1)
    {
        ypos += dy;
        for (int z = app.startZ; z < app.startZ + app.NOL; z++)
        {
            int inner = 0;
            for (int x = xpos - dx; x <= xpos - dx + (app.xmax - app.xmin); x++)
            {
                if (NoC[x][ypos][z].isFree != -1 || x < 0 || x >= Gw || ypos >= Gl || ypos <= -1)
                {
                    mark = 1;
                    inner = 1;
                    break;
                }
            }
            if (inner == 1)
                break;
        }
        if (mark == 1 || ypos >= Gl || ypos <= -1)
            break;
        tempyXY++;
    }
    pathXY.first = tempxXY;
    pathXY.second = tempyXY;

    xpos, ypos;
    if (dx == -1)
    {
        xpos = app.xmin;
    }
    else if (dx == 1)
    {
        xpos = app.xmax;
    }
    if (dy == -1)
    {
        ypos = app.ymin;
    }
    else if (dy == 1)
    {
        ypos = app.ymax;
    }

    int tempxYX = 0, tempyYX = 0;
    mark = 0;
    while (1)
    {
        ypos += dy;
        for (int z = app.startZ; z < app.startZ + app.NOL; z++)
        {
            int inner = 0;
            for (int x = app.xmin; x <= xpos - dx + (app.xmax - app.xmin); x++)
            {
                if (NoC[x][ypos][z].isFree != -1 || x < 0 || x >= Gw || ypos >= Gl || ypos <= -1)
                {
                    mark = 1;
                    inner = 1;
                    break;
                }
            }
            if (inner == 1)
                break;
        }
        if (mark == 1 || ypos >= Gl || ypos <= -1)
            break;
        tempyYX++;
    }
    mark = 0;
    while (1)
    {
        xpos += dx;
        for (int z = app.startZ; z < app.startZ + app.NOL; z++)
        {
            int inner = 0;
            for (int y = ypos - dy; y <= ypos - dy + (app.ymax - app.ymin); y++)
            {
                if (NoC[xpos][y][z].isFree != -1 || y < 0 || y >= Gl || xpos >= Gw || xpos <= -1)
                {
                    mark = 1;
                    inner = 1;
                    break;
                }
            }
            if (inner == 1)
                break;
        }
        if (mark == 1 || xpos >= Gw || xpos <= -1)
            break;
        tempxYX++;
    }
    pathYX.first = tempxYX;
    pathYX.second = tempyYX;

    return {pathXY, pathYX};
}

void migrateApplication(Application &app, pair<int, int> path, pair<int, int> dir)
{
    int startZ = app.startZ;
    for (int z = startZ; z < startZ + app.NOL; z++)
    {
        for (int y = app.ymin; y <= app.ymax; y++)
        {
            for (int x = app.xmin; x <= app.xmax; x++)
            {
                int newx = x + (dir.first * path.first), newy = y + (dir.second * path.second);
                NoC[x][y][z].isFree = -1;
                NoC[newx][newy][z].isFree = app.id;
            }
        }
    }
}

bool compare(const Application &a, const Application &b)
{
    return a.tasks.size() > b.tasks.size();
}

void defragmentation(vector<Application> &apps)
{
    double F = 1.0 - (double)calculateCenterFreeCores() / (Gw * Gl * Gh);
    const double FTH = 0.5;

    if (F > FTH)
    {
        sort(apps.begin(), apps.end(), compare);
        for (auto &app : apps)
        {
            int x = 0, y = 0;
            int RE = Gw - app.xmax;
            int RW = app.xmin;
            int RN = Gl - app.ymax;
            int RS = app.ymin;
            if (RN >= RS && RE >= RW)
            {
                x = -1, y = -1;
            }
            else if (RN < RS && RE >= RW)
            {
                x = -1, y = 1;
            }
            else if (RN >= RS && RE < RW)
            {
                x = 1, y = -1;
            }
            else if (RN < RS && RE < RW)
            {
                x = 1, y = 1;
            }
            pair<pair<int, int>, pair<int, int>> paths = findVirtualMigrationPaths(app, x, y);
            pair<int, int> path;
            if (paths.first.first + paths.first.second > paths.second.first + paths.second.second)
            {
                path = paths.first;
            }
            else
            {
                path = paths.second;
            }
            migrateApplication(app, path, {x, y});
        }
    }
}
int main()
{
    // vector<Application> apps = {
    //     {1, {1, 2, 3, 4, 5}, {{1, 2, 4}, {1, 3, 4}, {2, 4, 4}, {4, 5, 4}}, {3, 4, 5, 3}},
    //     {2, {3, 4, 2, 3, 1}, {{2, 4, 3}, {3, 2, 4}, {5, 2, 3}}, {6, 2, 2}},
    //     {3, {2, 3, 9, 1}, {{3, 4, 1}, {1, 4, 8}}, {4, 9, 2}},
    //     {4, {3, 6, 1, 7}, {{1, 4, 2}, {2, 1, 4}, {3, 1, 5}}, {2, 3, 6, 8}}};
    // apps[0].NOL = 1, apps[0].MD = 0;
    // apps[1].NOL = 1, apps[1].MD = 1;
    // apps[2].NOL = 1, apps[2].MD = 0;
    // apps[3].NOL = 1, apps[3].MD = 1;

    // apps[0].xmax = 2, apps[0].xmin = 0;
    // apps[0].ymax = 1, apps[0].xmin = 0;

    // apps[1].xmax = 4, apps[1].xmin = 2;
    // apps[1].ymax = 1, apps[1].ymin = 0;

    // apps[2].xmax = 3, apps[2].xmin = 2;
    // apps[2].ymax = 2, apps[2].ymin = 1;

    // apps[3].xmax = 2, apps[3].xmin = 1;
    // apps[3].ymax = 2, apps[3].ymin = 1;

    // apps[3].startZ = 1;
    // apps[3].startY = apps[3].ymin;
    // apps[3].startX = apps[3].xmin;

    // NoC[0][0][0].isFree = 1;
    // NoC[1][0][0].isFree = 1;
    // NoC[2][0][0].isFree = 1;
    // NoC[0][1][0].isFree = 1;
    // NoC[1][1][0].isFree = 1;

    // NoC[4][0][1].isFree = 2;
    // NoC[3][0][1].isFree = 2;
    // NoC[2][0][1].isFree = 2;
    // NoC[4][1][1].isFree = 2;
    // NoC[3][1][1].isFree = 2;

    // NoC[2][1][0].isFree = 3;
    // NoC[2][2][0].isFree = 3;
    // NoC[3][2][0].isFree = 3;
    // NoC[3][1][0].isFree = 3;

    // NoC[1][1][1].isFree = 4;
    // NoC[2][1][1].isFree = 4;
    // NoC[2][2][1].isFree = 4;
    // NoC[1][2][1].isFree = 4;

    // NoC[2][4][0].isFree = 5;
    // NoC[4][3][0].isFree = 5;

    vector<Application> apps = {
        {1, {1, 2, 3, 4, 5}, {{1, 2, 4}, {1, 3, 4}, {2, 4, 4}, {4, 5, 4}}, {3, 4, 5, 3}},
        {2, {3, 4, 2, 3, 1}, {{2, 4, 3}, {3, 2, 4}, {5, 2, 3}}, {6, 2, 2}},
        {3, {2, 3, 9, 1}, {{3, 4, 1}, {1, 4, 8}}, {4, 9, 2}}};

    for (int z = 0; z < Gh; ++z)
    {
        cout << "Layer " << z << ":" << endl;
        for (int y = 0; y < Gl; ++y)
        {
            for (int x = 0; x < Gw; ++x)
            {
                cout << NoC[x][y][z].isFree << " ";
                // cout << (NoC[x][y][z].isFree == -1 ? '.' : NoC[x][y][z].isFree);
                NoC[x][y][z].x = x;
                NoC[x][y][z].y = y;
                NoC[x][y][z].z = z;
            }
            cout << endl;
        }
        cout << endl;
    }

    int x = 0, y = 0;
    int RE = Gw - apps[3].xmax;
    int RW = apps[3].xmin;
    int RN = Gl - apps[3].ymax;
    int RS = apps[3].ymin;
    cout << RE << " " << RW << " " << RN << " " << RS << endl;
    if (RN >= RS && RE >= RW)
    {
        x = -1, y = -1;
    }
    else if (RN < RS && RE >= RW)
    {
        x = -1, y = 1;
    }
    else if (RN >= RS && RE < RW)
    {
        x = 1, y = -1;
    }
    else if (RN < RS && RE < RW)
    {
        x = 1, y = 1;
    }
    cout << "x" << x << "y" << y << endl;
    pair<pair<int, int>, pair<int, int>> paths = findVirtualMigrationPaths(apps[3], x, y);
    cout << paths.first.first << " " << paths.first.second << endl;
    cout << paths.second.first << " " << paths.second.second << endl;
    migrateApplication(apps[3], paths.first, {x, y});
    for (int z = 0; z < Gh; ++z)
    {
        cout << "Layer " << z << ":" << endl;
        for (int y = 0; y < Gl; ++y)
        {
            for (int x = 0; x < Gw; ++x)
            {
                cout << NoC[x][y][z].isFree << " ";
                // cout << (NoC[x][y][z].isFree == -1 ? '.' : NoC[x][y][z].isFree);
                // NoC[x][y][z].x = x;
                // NoC[x][y][z].y = y;
                // NoC[x][y][z].z = z;
            }
            cout << endl;
        }
        cout << endl;
    }
    // defragmentation(apps);
}
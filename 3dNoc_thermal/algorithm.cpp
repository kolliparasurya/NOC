#include<bits/stdc++.h>
using namespace std;

const int Gw = 4; 
const int Gl = 4;
const int Gh = 4; 

struct Application {
    int id;
    int numTasks;
    double avgCommVolume;
    int NOL; 
    int MD; 
};

struct Core {
    bool isFree = true;
    int x, y, z;
};

Core NoC[Gw][Gl][Gh];

void findCoreRegionShape(vector<Application>& apps) {
    for (auto &app : apps) {
        // Heuristic: Use ceil(sqrt(numTasks)) (capped by Gh) as the number of layers.
        app.NOL = min(Gh, (int)ceil(sqrt(app.numTasks)));
        app.MD = 0; 
    }
}


bool findCoreRegionLocation(Application &app, int cornerIndex) {
    int width = ceil(sqrt(app.numTasks / (double)app.NOL));
    int length = ceil(sqrt(app.numTasks / (double)app.NOL));
    
    int startX, startY, startZ, dirX, dirY;
    if (cornerIndex % 4 == 0) {
        startX = 0; startY = 0; startZ = app.MD;
        dirX = 1; dirY = 1;
    } else if (cornerIndex % 4 == 1) {
        startX = Gw - 1; startY = 0; startZ = app.MD;
        dirX = -1; dirY = 1;
    } else if (cornerIndex % 4 == 2) {
        startX = Gw - 1; startY = Gl - 1; startZ = app.MD;
        dirX = -1; dirY = -1;
    } else { 
        startX = 0; startY = Gl - 1; startZ = app.MD;
        dirX = 1; dirY = -1;
    }
    
    for (int y = startY; (dirY > 0 ? y < Gl : y >= 0); y += dirY) {
        for (int x = startX; (dirX > 0 ? x < Gw : x >= 0); x += dirX) {
            int available = 0;
            for (int z = startZ; z < startZ + app.NOL; ++z) {
                for (int dy = 0; dy < length; ++dy) {
                    for (int dx = 0; dx < width; ++dx) {
                        int nx = x + dx * dirX;
                        int ny = y + dy * dirY;
                        if (nx >= 0 && nx < Gw && ny >= 0 && ny < Gl && NoC[nx][ny][z].isFree) {
                            available++;
                        }
                    }
                }
            }
            if (available < app.numTasks)
                continue; 
            
            int assigned = 0;
            for (int z = startZ; z < startZ + app.NOL && assigned < app.numTasks; ++z) {
                for (int dy = 0; dy < length && assigned < app.numTasks; ++dy) {
                    for (int dx = 0; dx < width && assigned < app.numTasks; ++dx) {
                        int nx = x + dx * dirX;
                        int ny = y + dy * dirY;
                        if (nx >= 0 && nx < Gw && ny >= 0 && ny < Gl && NoC[nx][ny][z].isFree) {
                            NoC[nx][ny][z].isFree = false;
                            assigned++;
                        }
                    }
                }
            }
            return true;
        }
    }
    return false;
}

vector<Core> lineFreeCoreCount(Core current, char direction) {
    vector<Core> freeCores;
    int dx = 0, dy = 0;
    if (direction == 'X')
        dx = 1;
    else if (direction == 'Y')
        dy = 1;
    
    for (int i = 1; i < max(Gw, Gl); ++i) {
        int nx = current.x + i * dx;
        int ny = current.y + i * dy;
        if (nx >= 0 && nx < Gw && ny >= 0 && ny < Gl && NoC[nx][ny][current.z].isFree)
            freeCores.push_back(NoC[nx][ny][current.z]);
        else
            break;
    }
    for (int i = 1; i < max(Gw, Gl); ++i) {
        int nx = current.x - i * dx;
        int ny = current.y - i * dy;
        if (nx >= 0 && nx < Gw && ny >= 0 && ny < Gl && NoC[nx][ny][current.z].isFree)
            freeCores.push_back(NoC[nx][ny][current.z]);
        else
            break;
    }
    return freeCores;
}

int calculateCenterFreeCores() {
    int centerFreeCores = 0;
    for (int z = 0; z < Gh; ++z) {
        Core centerCore = NoC[Gw / 2][Gl / 2][z];
        vector<Core> xFreeCores = lineFreeCoreCount(centerCore, 'X');
        for (auto& core : xFreeCores) {
            vector<Core> yFreeCores = lineFreeCoreCount(core, 'Y');
            centerFreeCores += yFreeCores.size();
        }
    }
    return centerFreeCores;
}

pair<vector<Core>, vector<Core>> findVirtualMigrationPaths(Application& app, Core refCore) {
    vector<Core> pathXY, pathYX;
    Core current = refCore;
    
    while (current.x >= 0 && current.x < Gw &&
           current.y >= 0 && current.y < Gl &&
           NoC[current.x][current.y][current.z].isFree) {
        pathXY.push_back(current);
        current.x++;
    }
    current = refCore;
    while (current.y >= 0 && current.y < Gl &&
           NoC[current.x][current.y][current.z].isFree) {
        pathXY.push_back(current);
        current.y++;
    }
    
    current = refCore;
    while (current.y >= 0 && current.y < Gl &&
           NoC[current.x][current.y][current.z].isFree) {
        pathYX.push_back(current);
        current.y++;
    }
    current = refCore;
    while (current.x >= 0 && current.x < Gw &&
           NoC[current.x][current.y][current.z].isFree) {
        pathYX.push_back(current);
        current.x++;
    }
    
    return {pathXY, pathYX};
}

void defragmentation(vector<Application>& apps) {
    double F = 1.0 - (double)calculateCenterFreeCores() / (Gw * Gl * Gh);
    const double FTH = 0.5; 
    
    if (F > FTH) {
        for (int z = 0; z < Gh; ++z) {
            for (int y = 0; y < Gl; ++y) {
                for (int x = 0; x < Gw; ++x) {
                    NoC[x][y][z].isFree = true;
                }
            }
        }
        int cornerIndex = 0;
        for (auto& app : apps) {
            findCoreRegionLocation(app, cornerIndex % 4);
            cornerIndex++;
        }
    }
}

int main() {
    vector<Application> apps = {
        {0, 4, 10.0},
        {1, 3, 8.0},
        {2, 2, 8.0},
        {3, 5, 10.0},
        {4, 3, 10.0}
    };
    
    findCoreRegionShape(apps);
    
    int cornerIndex = 0;
    for (auto& app : apps) {
        findCoreRegionLocation(app, cornerIndex % 4);
        cornerIndex++;
    }
    
    defragmentation(apps);
    
    cout << "Final state of the NoC:" << endl;
    for (int z = 0; z < Gh; ++z) {
        cout << "Layer " << z << ":" << endl;
        for (int y = 0; y < Gl; ++y) {
            for (int x = 0; x < Gw; ++x) {
                cout << (NoC[x][y][z].isFree ? '.' : '1');
            }
            cout << endl;
        }
        cout << endl;
    }
    return 0;
}

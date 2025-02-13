#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

const int Gw = 8;
const int Gl = 8;
const int Gh = 4;

struct Application {
    int numTasks;       
    int NOL;            
    int MD;             
    int width;          
    int length;         // Length of the core region
};

// Function to calculate the number of free cores in a given direction
vector<int> lineFreeCoreCount(int startX, int startY, int layer, const vector<vector<vector<bool>>>& freeCores, char direction) {
    vector<int> freeCoreArray;
    if (direction == 'X') {
        // Search along X+ direction
        for (int x = startX; x < Gw && freeCores[layer][startY][x]; ++x) {
            freeCoreArray.push_back(x);
        }
    } else if (direction == 'Y') {
        // Search along Y+ direction
        for (int y = startY; y < Gl && freeCores[layer][y][startX]; ++y) {
            freeCoreArray.push_back(y);
        }
    }
    return freeCoreArray;
}

// Function to find a free core region for an application
bool findFreeCoreRegion(int& startX, int& startY, int& startLayer, const Application& app, 
                         const vector<vector<vector<bool>>>& freeCores) {
    for (int layer = app.MD; layer + app.NOL <= Gh; ++layer) {
        for (int y = 0; y + app.length <= Gl; ++y) {
            for (int x = 0; x + app.width <= Gw; ++x) {
                bool found = true;
                for (int l = layer; l < layer + app.NOL; ++l) {
                    for (int j = y; j < y + app.length; ++j) {
                        for (int i = x; i < x + app.width; ++i) {
                            if (!freeCores[l][j][i]) {
                                found = false;
                                break;
                            }
                        }
                        if (!found) break;
                    }
                    if (!found) break;
                }
                if (found) {
                    startX = x;
                    startY = y;
                    startLayer = layer;
                    return true;
                }
            }
        }
    }
    return false;
}

// Algorithm 2: Finding the Exact Locations of Core Regions
void findExactLocations(vector<Application>& applications, vector<vector<vector<bool>>>& freeCores) {
    // Sort applications by the number of tasks in descending order
    sort(applications.begin(), applications.end(), [](const Application& a1, const Application& a2) {
        return a1.numTasks > a2.numTasks;
    });

    int CI = 0; // Corner index for round-robin placement
    for (auto& app : applications) {
        // Determine the search starting point and direction based on the corner index
        int startX = 0, startY = 0, startLayer = app.MD;
        if (CI == 0) {
            startX = 0;
            startY = 0;
        } else if (CI == 1) {
            startX = Gw - app.width;
            startY = 0;
        } else if (CI == 2) {
            startX = 0;
            startY = Gl - app.length;
        } else if (CI == 3) {
            startX = Gw - app.width;
            startY = Gl - app.length;
        }

        // Find a free core region for the application
        if (!findFreeCoreRegion(startX, startY, startLayer, app, freeCores)) {
            cerr << "Error: No free core region found for application!" << endl;
            continue;
        }

        // Mark the cores as occupied
        for (int l = startLayer; l < startLayer + app.NOL; ++l) {
            for (int j = startY; j < startY + app.length; ++j) {
                for (int i = startX; i < startX + app.width; ++i) {
                    freeCores[l][j][i] = false;
                }
            }
        }

        // Update the corner index for the next application
        CI = (CI + 1) % 4;
    }
}

int main() {
    // Example applications
    vector<Application> applications = {
        {8, 2, 1, 2, 2}, // Application 1: 8 tasks, NOL=2, MD=1, width=2, length=2
        {6, 1, 0, 2, 2}  // Application 2: 6 tasks, NOL=1, MD=0, width=2, length=2
    };

    // Initialize the free cores in the NoC (true means free, false means occupied)
    vector<vector<vector<bool>>> freeCores(Gh, vector<vector<bool>>(Gl, vector<bool>(Gw, true)));

    // Find the exact locations for the core regions
    findExactLocations(applications, freeCores);

    // Output the results
    cout << "Core regions mapped successfully!" << endl;
    return 0;
}
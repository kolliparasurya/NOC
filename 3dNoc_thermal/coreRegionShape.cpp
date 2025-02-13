#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <limits>

using namespace std;

struct TreeNode {
    vector<int> NOL; 
    vector<int> MD;  
    double estimated_running_time; 
};

double calculateEstimatedRunningTime(const TreeNode& node, const vector<int>& taskCounts, const vector<double>& avgCommVolumes) {

    double totalERT = 0.0;
    for (size_t i = 0; i < node.NOL.size(); ++i) {
        totalERT += taskCounts[i] + avgCommVolumes[i] + node.NOL[i] + node.MD[i];
    }
    return totalERT;
}

bool isFeasible(const TreeNode& node, const vector<int>& taskCounts, int NoCWidth, int NoCLength, int NoCHeight) {
    for (size_t i = 0; i < node.NOL.size(); ++i) {
        int tasksPerLayer = ceil(static_cast<double>(taskCounts[i]) / node.NOL[i]);
        if (tasksPerLayer > NoCWidth * NoCLength || node.MD[i] + node.NOL[i] > NoCHeight) {
            return false;
        }
    }
    return true;
}

TreeNode branchAndBound(queue<TreeNode>& WQ, const vector<int>& taskCounts, const vector<double>& avgCommVolumes,
                         int NoCWidth, int NoCLength, int NoCHeight) {
    double minOverallRunningTime = numeric_limits<double>::infinity();
    TreeNode bestNode;

    while (!WQ.empty()) {
        TreeNode Nq = WQ.front();
        WQ.pop();

        if (Nq.NOL.size() == taskCounts.size()) { // Leaf node
            if (Nq.estimated_running_time < minOverallRunningTime) {
                minOverallRunningTime = Nq.estimated_running_time;
                bestNode = Nq;
            }
        } else {
            size_t nextAppIndex = Nq.NOL.size();
            int NOL_MIN = ceil(static_cast<double>(taskCounts[nextAppIndex]) / (NoCWidth * NoCLength));
            int NOL_MAX = min(NoCHeight, taskCounts[nextAppIndex]);
            int MD_MIN = 0;
            int MD_MAX = NoCHeight - NOL_MIN;

            for (int NOLi = NOL_MIN; NOLi <= NOL_MAX; ++NOLi) {
                for (int MDi = MD_MIN; MDi <= MD_MAX; ++MDi) {
                    TreeNode child = Nq;
                    child.NOL.push_back(NOLi);
                    child.MD.push_back(MDi);
                    child.estimated_running_time = calculateEstimatedRunningTime(child, taskCounts, avgCommVolumes);

                    if (isFeasible(child, taskCounts, NoCWidth, NoCLength, NoCHeight)) {
                        if (child.estimated_running_time < minOverallRunningTime) {
                            WQ.push(child);
                        }
                    }
                }
            }
        }
    }

    return bestNode;
}

pair<vector<int>, vector<int>> findCoreRegionShape(const vector<int>& taskCounts, const vector<double>& avgCommVolumes,
                                                   int NoCWidth, int NoCLength, int NoCHeight) {
    queue<TreeNode> WQ;
    TreeNode root;
    root.estimated_running_time = 0.0;
    WQ.push(root);

    TreeNode bestNode = branchAndBound(WQ, taskCounts, avgCommVolumes, NoCWidth, NoCLength, NoCHeight);

    return {bestNode.MD, bestNode.NOL};
}

int main() {
    vector<int> taskCounts = {8, 6}; 
    vector<double> avgCommVolumes = {10.5, 8.2};
    int NoCWidth = 8, NoCLength = 8, NoCHeight = 4;
    pair<vector<int>, vector<int>> result = findCoreRegionShape(taskCounts, avgCommVolumes, NoCWidth, NoCLength, NoCHeight);
    cout << "MD values: ";
    for (int md : result.first)cout << md << " ";
    cout << endl;
    cout << "NOL values: ";
    for (int nol : result.second)cout << nol << " ";
    cout << endl;
}
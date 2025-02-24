# NOC: 3D NOC Paper Implementation

This repository contains the code implementation related to the 3D NOC paper. The project integrates techniques for thermal mapping and reinforcement learning in 3D environments, as described in the paper.

### Repository Structure

- **3dNoc_thermal/**  
   Implementation of "On Runtime Communication and Thermal-Aware Application Mapping and Defragmentation in 3D NoC Systems" research paper regarding the mapping the applications to cores on themral constraint using defragmentation technique.
  
- **RL/**  
  Implementatino of "NoC Application Mapping Optimization Using Reinforcement Learning" (https://doi.org/10.1145/3510381) mapping the cores to routers with reinforcement learning.
  
- **.gitignore**  
  Specifies files and directories to be ignored by Git.
  
- **.vscode/**  
  Contains VS Code settings to standardize development across different machines.

### Prerequisites for RL

- Python 3.7 or higher
- Git
- (Optional) A virtual environment tool (e.g., `venv` or `conda`)

### Prerequisites for 3dNoc_thermal

 -CPP compiler install

### Cloning the Repository

  Clone the repository to your local machine:
  ```bash
  git clone https://github.com/kolliparasurya/NOC.git
  cd NOC
  ```

### Create the virtual environment
```bash
python3 -m venv venv

# Activate on Linux/Mac
source venv/bin/activate

# Activate on Windows
venv\Scripts\activate
```

### Program Usage

 #### 3dNoc_thermal 
   ```bash
   cd 3dNoc_thermal
   g++ program.cpp
   ```




<!--<h1 align="center">
  <br>
    VLSI Physical Design Automation - global routing
  <br>
</h1>


## Project Goal
* The routing problem can be divided into two steps:
global routing and detailed routing. During global routing, nets are connected on a
coarse‐grain grid graph with capacity constraints. Then detailed routing follows the
solution obtained in global routing to find the exact routing solution. The quality of
global routing affects timing, power and density in the chip area, and thus global
routing is a very important stage in the design cycle.
* In this project, the goal is to implement an existing algorithm or develop
your own algorithm to solve the global routing problem with a set of 2-pin nets.

## Methodology
* The algorithm I implemented can be divide into the following steps:
    1. First of all, I use "maze routing" algorithm route all nets. If there is an overflow situation on the route path, overflow global edges will be the obstacles in maze routing algorithm. By using this algorithm, we can not only reduce the overflow, but also make the wirelength as short as possible.
    2. If there still have some overflow situations after first step, I will rip up all overflow nets, and reroute them with maze routing algorithm just as first step, but in different order.
    3. I will continue to perform the second step, until there is no overflow, or the number of iterations is greater than 20.

## Result
* Test case: ibm01
<br>
<p align="center">
<img src="https://github.com/rrrjjj2019/VLSI-Physical-Design-Automation-global-routing/blob/master/01.JPG" width="500" style="margin-right:5px; border: 1px solid #ccc;" />
</p>
<br>

* Test case: ibm04
<br>
<p align="center">
<img src="https://github.com/rrrjjj2019/VLSI-Physical-Design-Automation-global-routing/blob/master/04.JPG" width="600" style="margin-right:5px; border: 1px solid #ccc;" />
</p>
<br>


* Legend
<br>
<p align="center">
<img src="https://github.com/rrrjjj2019/VLSI-Physical-Design-Automation-global-routing/blob/master/legend.JPG" width="120" style="margin-right:5px; border: 1px solid #ccc;" />
</p>
<br>
-->

<h1 align="center">
  🏗️ VLSI Physical Design Automation – Global Routing
</h1>

## 📖 Overview
This project explores **global routing algorithms** in VLSI physical design automation (EDA).  
The routing problem in chip design consists of two major phases:  
1. **Global Routing** – Connect nets on a coarse-grain grid graph under capacity constraints.  
2. **Detailed Routing** – Refine the global routing solution into exact wire paths.  

Since the quality of **global routing** strongly impacts **timing, power, and routing congestion**, it is a critical stage in the chip design cycle.  
The objective of this project is to **implement and experiment with algorithms for global routing** using benchmark test cases.

---

## 🎯 Project Goals
- Implement an algorithm for **congestion-aware global routing**.  
- Evaluate the trade-offs between **wirelength reduction** and **overflow minimization**.  
- Gain hands-on experience in **physical design CAD tool development**.  

---

## 🛠️ Methodology
The routing flow is based on a **maze routing strategy with iterative refinement**:

1. **Initial Routing**  
   - Use the maze routing algorithm to connect all nets.  
   - If overflow occurs, treat overflowed edges as obstacles in subsequent iterations.  

2. **Rip-Up and Reroute**  
   - Rip up all nets that pass through overflowed edges.  
   - Reroute them using maze routing, but with different net ordering.  

3. **Iteration**  
   - Repeat Step 2 until either:  
     - No overflow remains, or  
     - The maximum number of iterations (20) is reached.  

This approach balances **congestion relief** and **wirelength optimization**.

---

## 📊 Results
Benchmarks from the **IBM standard test cases** were used for validation.

- **Test Case: ibm01**  
  <p align="center">
    <img src="https://github.com/rrrjjj2019/VLSI-Physical-Design-Automation-global-routing/blob/master/01.JPG" width="500" style="margin:5px; border:1px solid #ccc;" />
  </p>

- **Test Case: ibm04**  
  <p align="center">
    <img src="https://github.com/rrrjjj2019/VLSI-Physical-Design-Automation-global-routing/blob/master/04.JPG" width="600" style="margin:5px; border:1px solid #ccc;" />
  </p>

- **Legend**  
  <p align="center">
    <img src="https://github.com/rrrjjj2019/VLSI-Physical-Design-Automation-global-routing/blob/master/legend.JPG" width="120" style="margin:5px; border:1px solid #ccc;" />
  </p>

---

## 🔑 Key Contributions
- Implemented a **congestion-aware maze routing algorithm** for VLSI global routing.  
- Applied **rip-up and reroute** techniques to iteratively reduce overflow.  
- Validated performance on **industry-standard IBM benchmarks**.  
- Demonstrated practical understanding of **EDA algorithm design**.  

---

## 📚 Future Work
- Compare performance against other algorithms (e.g., Steiner tree, multicommodity flow).  
- Extend framework toward **detailed routing** integration.  
- Explore ML-driven congestion prediction for better net ordering.  

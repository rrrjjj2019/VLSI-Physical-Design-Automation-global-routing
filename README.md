<h1 align="center">
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


v2v: ns-3 extension for vehicular communications
========================================================


### An overview
v2v module is a new ns-3 extension containing a clustering algorithm implementation for VANETs and one example on how to use the new model.

*The code is tested in 3.21 version of ns-3 simulator*


### Build v2v
Copy and paste the v2v package under src/ folder of the ns-3 simulator.

type the command 

`./waf configure --enable-examples` 

followed by 

`./waf`

in the the root directory of the ns-3 simulator. The files built will be copied in the build/ directory.


### Run the v2v example
type the command 

`./waf --run v2v-clustering-example`

*The source code of the example can be found in the examples/ directory*

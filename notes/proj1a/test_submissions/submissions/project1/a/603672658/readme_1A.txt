Wei-Yen (James) Chung
603672658
zphantom@ucla.edu

-------------------------------------------------------------------------------------

1. Calculate simple arithmetic problems (+-*/).
2. Support negatives and floating points.
3. No parentheses or spaces allowed.

-------------------------------------------------------------------------------------

Problems Solved:

1. Tracked down div-by-zero problem. (Print out notification - Invalid Input)
2. Solved minus negative problem.
3. Allowed decimal points without following or followed by a number. (At least one side has to have a number)
   ie. "80." and ".009" are allowed, but not ".".
4. Allowed adding unnecessary 0s before or after numbers.
   ie. "00024" and "45.900" are allowed.
5. Solved problem that adding 0s before integers causes the eval() function to treat it as octal.
   ie. "064" equals 64 instead of 52.
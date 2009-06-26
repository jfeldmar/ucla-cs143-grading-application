Name: John (An Gyun) Gang
SID: 703-511-311
EMAIL: johnaggang@gmail.com

-It is now modifies so that it takes in negative values.

It seems like dividing by zero error is not handled correctly. For example, 10.5+20*3/25/0 is handled as if the last zero and the division doesn't exist. I handled it as "undefined".

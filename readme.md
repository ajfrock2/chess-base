Added Pawn, King, and Knight Movements

Kings and Knights were pretty easy, didn't take much time. 
Pawns on the other hand were a struggle. 
Took a while to grasp my head around, but I now at least understand how all the code is working. 
Spent a good bit of time refractoring as well, hopefully this will help me in the long run.
Definetily more readable now.

Added Rook, Bishop, and Queen movement. Added randomized AI.

Magicbitboards.h had a lot of the work already done, but it sitll took a little while to understand. 
Had a bit of trouble working with the combination of pointers, refrence, and actual objects. 
Using the state string for all the calculations had a fair bit of issues.
I mentally mixed up which pieces were black or white a few times, wasting time. (K vs k)
Works with a depth of 3 fast, 4 is bearable. Hopefully wont get that much worse as I add actual board evaluation logic
Had some hiccups getting the AI to play smart. Ultimately was caused by a bug with later depths and generating moves for the wrong player.